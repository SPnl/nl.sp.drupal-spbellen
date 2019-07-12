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
  $contact_id = FALSE;
  $fields = array_keys(spbellen_contact_fields());
  foreach ($fields as $field) {
    if (!empty($contact_data[$field])) {
      $values[$field] = $contact_data[$field];
    }
  }

  $values['phone'] = spbellen_cleanup_phone($values['phone']);

  if (spbellen_valid_phone($values['phone'])) {
    // Get contact id.
    if (!empty($values['contact_id'])) {
      $contact_id = $values['contact_id'];
    }
    elseif (!empty($values['crm_contact_id'])) {
      // Get contact_id from crm_contact_id.
      $result = db_query("SELECT contact_id FROM {spbellen_contact_data} WHERE crm_contact_id = :crm_contact_id", array(':crm_contact_id' => $values['crm_contact_id']))->fetchField();
      if (!empty($result)) {
        $contact_id = $result;
      }
    }
    elseif (!empty($values['phone']) && empty($values['crm_contact_id'])) {
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
    elseif (!empty($values['crm_contact_id'])) {
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
function spbellen_import_merge_group($title, $importer, $external_group_id) {
  if (!empty($title) && !empty($importer) && !empty($external_group_id)) {
    db_merge('spbellen_groups')
      ->key(array(
        'external_group_id' => $external_group_id,
      ))
      ->fields(array(
        'title' => $title,
        'importer' => $importer,
      ))
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
