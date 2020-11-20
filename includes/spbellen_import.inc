<?php

/**
 * @file
 * Import helper functions.
 */

/**
 * Import contact in campaign.
 *
 * @param int $group_id
 *   Group id.
 * @param array $contact_data
 *   See spbellen_contact_fields() for keys.
 */
function spbellen_import_contact($import_id, $group_id, array $contact_data) {
  if (!empty($contact_data['phone'])) {
    $contact_id = FALSE;
    $fields = array_keys(spbellen_contact_fields());
    $gender_mapping = spbellen_import_gender_mapping();
    foreach ($fields as $field) {
      if (!empty($contact_data[$field])) {
        if ($field === 'gender') {
          // Map different import gender values to db gender values.
          $db_gender_value = spbellen_import_db_map_gender_value($contact_data['gender']);
          if (!empty($db_gender_value)) {
            $values['gender'] = $db_gender_value;
          }
        }
        else {
          $values[$field] = $contact_data[$field];
        }
      }
    }

    $values['phone'] = spbellen_cleanup_phone($values['phone']);

    if (spbellen_valid_phone($values['phone'])) {
      // Get contact id.
      if (!empty($values['contact_id'])) {
        $contact_id = $values['contact_id'];
      }
      if (empty($contact_id) && !empty($values['crm_contact_id'])) {
        // Get contact_id from crm_contact_id.
        $result = db_query("SELECT contact_id FROM {spbellen_contact_data} WHERE crm_contact_id = :crm_contact_id", array(':crm_contact_id' => $values['crm_contact_id']))->fetchField();
        if (!empty($result)) {
          $contact_id = $result;
        }
      }
      if (empty($contact_id) && !empty($values['email'])) {
        // Get contact_id from phone.
        $result = db_query("SELECT contact_id FROM {spbellen_contact_data} WHERE email LIKE :email", array(':email' => $values['email']))->fetchField();
        if (!empty($result)) {
          $contact_id = $result;
        }
      }
      if (empty($contact_id) && !empty($values['phone'])) {
        // Get contact_id from phone.
        $result = db_query("SELECT contact_id FROM {spbellen_contact_data} WHERE phone LIKE :phone", array(':phone' => '%' . db_like(substr($values['phone'], -8))))->fetchField();
        if (!empty($result)) {
          $contact_id = $result;
        }
      }
      // Store contact data.
      if (empty($contact_id)) {
        $contact_id = db_insert('spbellen_contact_data')
          ->fields($values)
          ->execute();
      }
      else {
        db_update('spbellen_contact_data')
          ->fields($values)
          ->condition('contact_id', $contact_id)
          ->execute();
      }
      // Get campaigns by group.
      $query = "SELECT campaign_id FROM {spbellen_group_campaign_relation} WHERE group_id = :group_id";
      $result = db_query($query, array(':group_id' => $group_id));
      if ($result) {
        while ($campaign_id = $result->fetchField()) {
          // Add target to campaign.
          $target_id = db_query("SELECT id FROM {spbellen_targets} WHERE contact_id = :contact_id AND campaign_id = :campaign_id", array(':contact_id' => $contact_id, ':campaign_id' => $campaign_id))->fetchField();
          if (empty($target_id)) {
            $target_id = db_insert('spbellen_targets')
              ->fields(array(
                'campaign_id' => $campaign_id,
                'state' => 1,
                'occupied' => 0,
                'contact_id' => $contact_id,
              ))
              ->execute();
          }
          // Add target import relation.
          db_merge('spbellen_target_import_relation_new')
            ->key(array(
              'target_id' => $target_id,
              'import_id' => $import_id,
            ))
            ->execute();
        }
      }
      return $contact_id;
    }
  }
  return FALSE;
}

/**
 * Gets last imported id within external group for campaign or returns 0.
 */
function spbellen_import_get_last_imported($external_group_id) {
  $query = "SELECT external_last_imported_id FROM {spbellen_groups} WHERE external_group_id = :external_group_id";
  $record = db_query($query, array(':external_group_id' => $external_group_id))->fetchField();
  return empty($record) ? 0 : $record;
}

/**
 * Adds group to campaign.
 */
function spbellen_import_insert_group_campaign_relation($group_id, $campaign_id) {
  if (!empty($group_id) && !empty($campaign_id)) {
    db_merge('spbellen_group_campaign_relation')
      ->key(array(
        'group_id' => $group_id,
        'campaign_id' => $campaign_id,
      ))
      ->execute();
    return TRUE;
  }
  return FALSE;
}

/**
 * Merge group.
 */
function spbellen_import_merge_group($importer, $external_group_id, $title = NULL, $auto_import = NULL) {
  if (!empty($importer) && !empty($external_group_id)) {
    $fields = array(
      'importer' => $importer,
    );
    if (isset($auto_import)) {
      $fields['auto_import'] = $auto_import;
    }
    if (!empty($title)) {
      $fields['title'] = $title;
    }
    db_merge('spbellen_groups')
      ->key(array(
        'external_group_id' => $external_group_id,
      ))
      ->fields($fields)
      ->execute();
    $query = "SELECT id FROM {spbellen_groups} WHERE external_group_id = :external_group_id";
    $group_id = db_query($query, array(':external_group_id' => $external_group_id))->fetchField();
    if (!empty($group_id)) {
      return $group_id;
    }
  }
  return FALSE;
}

/**
 * Insert import data.
 */
function spbellen_import_insert_import($group_id) {
  if (!empty($group_id)) {
    $import_id = db_insert('spbellen_imports')
      ->fields(array(
        'group_id' => $group_id,
        'date' => time(),
      ))
      ->execute();
    return $import_id;
  }
  return FALSE;
}

/**
 * Set auto import flag (0 or 1).
 */
function spbellen_import_set_auto_import($group_id, $status) {
  if (!empty($group_id) && in_array($status, array(0, 1))) {
    db_update('spbellen_groups')
      ->condition('id', $group_id)
      ->fields(array(
        'auto_import' => empty($status) ? 0 : 1,
      ))
      ->execute();
  }
}

/**
 * Map gender import values to db values.
 */
function spbellen_import_gender_mapping() {
  $gender_mapping = array(
    'Man' => array(
      'Man',
      'm',
    ),
    'Vrouw' => array(
      'Vrouw',
      'v',
    ),
    'Anders' => array(
      'Anders',
      'a',
      '?',
    ),
  );
  return $gender_mapping;
}


function spbellen_import_db_map_gender_value($gender_value) {
  $gender_mapping = spbellen_import_gender_mapping();
  foreach ($gender_mapping as $db_gender_value => $data_gender_values) {
    if (array_search($gender_value, $data_gender_values) !== FALSE) {
      return $db_gender_value;
    }
  }
  return FALSE;
}
