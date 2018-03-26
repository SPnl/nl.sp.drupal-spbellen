<?php

/**
 * Helper functions for spbellen civi importer module.
 */

/**
 * Get all but smart groups.
 */
function spbellen_civi_import_get_groups() {
  $groups = &drupal_static(__FUNCTION__);
  if (!isset($groups)) {

    if ($cache = cache_get('spbellen_civi_import_groups')) {
      $groups = $cache->data;
    }
    else {
      $groups = spbellen_civi_import_get_civi_groups('all_but_smart');
      cache_set('spbellen_civi_import_groups', $groups, 'cache', time() + 3600);
    }
  }
  return $groups;
}

/**
 * Get civi groups.
 */
function spbellen_civi_import_get_civi_groups() {
  $groups = array();
  set_time_limit(0);
  $spcivi = \SPCivi::getInstance();
  $params = array(
    'sequential'            => 1,
    'option.limit'          => 9999,
    'is_active'             => 1,
    'return' => "id, title",
    'saved_search_id' => array('IS NULL' => 1),
  );
  $groups_results = $spcivi->api('Group', 'get', $params);
  if (spbellen_check_result($groups_results, 'groepen', 'get', $params)) {
    if (!empty($groups_results['values'])) {
      foreach ($groups_results['values'] as $value) {
        if (
          empty($value['group_type']) ||
          (array_key_exists(0, $value['group_type']) && empty($value['group_type'][0])) ||
          in_array('2', $value['group_type'])
        ) {
          $groups[$value['id']] = $value['title'];
        }
      }
      asort($groups);
    }
    else {
      drupal_set_message('Error syncing civicrm groups, using saved data.', 'warning');
    }
  }
  return $groups;
}

/**
 * Return group autocomplete.
 */
function spbellen_civi_import_group_autocomplete($filter, $string) {
  $string = strtolower($string);
  $matches = array();
  if (strlen($string) > 2) {
    $matches = array();
    $groups = spbellen_civi_import_get_groups();
    foreach ($groups as $key => $group) {
      if (strpos(strtolower($group), $string) !== FALSE) {
        $matches[$group . ' (' . $key . ')'] = $group;
      }
      if (count($matches) > 10) {
        break;
      }
    }
  }

  // Return the result to the form in json.
  drupal_json_output($matches);
}

/**
 * Get batch.
 */
function spbellen_civi_import_get_targets_batch($campaign_id, $group_ids, $sync_id, &$context) {
  // Process by chunks.
  $chunk_size = 20;
  $group_id = $group_ids['id'];
  $group_name = $group_ids['name'];
  if (empty($context['results']['count'])) {
    $context['reults']['count'] = array();
  }

  set_time_limit(0);
  $spcivi = \SPCivi::getInstance();

  if (empty($context['sandbox'])) {
    // Get total number of contacts.
    $number_of_contacts = 0;
    $params = array(
      'group' => $group_id,
      'is_deleted' => 0,
      'status' => 'Added',
    );
    $results = $spcivi->api('Contact', 'getcount', $params);
    if (spbellen_check_result($results, 'contacten', 'get', $params)) {
      $number_of_contacts = $results['result'];
    }

    $context['sandbox'] = array();
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['current_contact'] = 0;
    $context['sandbox']['max'] = $number_of_contacts;
    $context['sandbox']['current_chunk'] = 0;
  }

  $params = array(
    'options' => array('limit' => $chunk_size, 'offset' => $context['sandbox']['progress']),
    'group' => $group_id,
    'include_non_menmbers' => 1,
  );
  $results = $spcivi->api('Contact', 'getspdata', $params);
  if (spbellen_check_result($results, 'contacten', 'get', $params)) {
    if (!empty($results['values'])) {
      foreach ($results['values'] as $key => $result) {
        // Process contact.
        $contact_id = $result['contact_id'];

        if ((isset($result['phone']) || isset($result['mobile'])) && empty($result['do_not_phone'])) {
          // Check if has phonenumer.
          if (!empty($result['mobile'])) {
            $result['phone'] = $result['mobile'];
          }

          $result['phone'] = preg_replace('/\D/', '', $result['phone']);
          $result['is_lid'] = $result['is_member'];
          $result['afdeling_id'] = empty($result['afdeling_code']) ? '' : $result['afdeling_code'];

          if (empty($context['results']['count'][$group_id])) {
            $context['results']['count'][$group_id] = 0;
          }
          $context['results']['count'][$group_id]++;
          spbellen_civi_import_process_target($campaign_id, $result, $sync_id);
        }

        // Stats.
        $context['results']['sync_id'] = $sync_id;
        $context['results']['campaign_id'] = $campaign_id;

        // Update our progress information.
        $context['sandbox']['progress']++;
        $context['sandbox']['current_contact'] = $contact_id;
      }

    }
    else {
      $context['finished'] = 1;
      return;
    }
  }
  $context['sandbox']['current_chunk']++;
  $context['message'] = ($context['sandbox']['current_chunk'] * $chunk_size) . '/' . $context['sandbox']['max'];
  $context['finished'] = ($context['sandbox']['progress'] / $context['sandbox']['max']);
}

/**
 * Process target.
 */
function spbellen_civi_import_process_target($campaign_id, $result, $sync_id) {
  $fields = array_keys(spbellen_contact_fields());
  foreach ($fields as $field) {
    if (!empty($result[$field])) {
      $values[$field] = $result[$field];
    }
  }

  $crm_contact_id = $values['contact_id'];
  unset($values['contact_id']);
  $values['crm_contact_id'] = $crm_contact_id;

  // Get notes.
  $notes = spbellen_civi_import_get_notes($crm_contact_id);
  if (!empty($notes)) {
    $notes_serialized = serialize($notes);
    $values['notes'] = $notes_serialized;
  }

  // Add target contact data.
  /*db_merge('spbellen_contact_data')
  ->key(array('crm_contact_id' => $crm_contact_id))
  ->fields($values)
  ->execute();*/
  if ($contact_id = db_query("SELECT contact_id FROM {spbellen_contact_data} WHERE crm_contact_id = :crm_contact_id", array(':crm_contact_id' => $crm_contact_id))->fetchField()) {
    db_update('spbellen_contact_data')
      ->fields($values)
      ->condition('contact_id', $contact_id)
      ->execute();
  }
  else {
    $contact_id = db_insert('spbellen_contact_data')
      ->fields($values)
      ->execute();
  }
  // Add target.
  db_merge('spbellen_targets')
    ->key(array(
      'contact_id' => $contact_id,
      'campaign_id' => $campaign_id,
    ))
    ->fields(array(
      'occupied' => 0,
      'sync_id' => $sync_id,
      'state' => 1,
    ))
    ->execute();
  // Delete contact from blacklist.
  db_delete('spbellen_blacklist')
    ->condition('contact_id', $contact_id)
    ->execute();
}

/**
 * Finish batch.
 */
function spbellen_civi_import_get_targets_batch_finished($success, $results, $operations) {
  if ($success) {
    drupal_set_message('Aantal opgehaalde contacten: ' . array_sum($results['count']));
    // To do: cleanup old syncs.
    $sync_id = $results['sync_id'];
    $campaign_id = $results['campaign_id'];
    $query = "UPDATE {spbellen_targets} ta SET ta.state = 0 WHERE ( sync_id < :sync_id OR sync_id IS NULL ) AND campaign_id = :campaign_id";
    $result = db_query($query, array(':sync_id' => $sync_id, ':campaign_id' => $campaign_id));
  }
  else {
    $error_operation = reset($operations);
    drupal_set_message(
      t('An error occurred while processing @operation with arguments : @args',
        array(
          '@operation' => $error_operation[0],
          '@args' => print_r($error_operation[0], TRUE),
        )
      ),
      'error'
    );
  }
}

/**
 * Get notes.
 */
function spbellen_civi_import_get_notes($crm_contact_id) {
  $notes = array();
  $civi_note_content_field_id = variable_get('spcivipush_note_content_field');
  $civi_note_use_field_id = variable_get('spcivipush_note_use_field');

  if (
    !empty($civi_note_content_field_id) &&
    !empty($civi_note_use_field_id)
  ) {
    set_time_limit(0);
    $spcivi = \SPCivi::getInstance();

    $params = array(
      'sequential' => 1,
      'entity_id' => $crm_contact_id,
    );
    $results = $spcivi->api('CustomValue', 'get', $params);
    if (spbellen_check_result($results, 'customvalues', 'get', $params)) {
      if (!empty($results['values'])) {
        foreach ($results['values'] as $value) {
          if ($value['id'] == $civi_note_content_field_id) {
            $note_content_values = $value;
          }
          if ($value['id'] == $civi_note_use_field_id) {
            foreach ($value as $value_key => $value_value) {
              if (filter_var($value_key, FILTER_VALIDATE_INT)) {
                if (in_array('telefoongesprek', $value_value)) {
                  $phone_note_keys[$value_key] = $value_key;
                }
              }
            }
          }
        }
        if (!empty($note_content_values)) {
          foreach ($note_content_values as $value_key => $value_value) {
            if (filter_var($value_key, FILTER_VALIDATE_INT)) {
              if (in_array($value_key, $phone_note_keys)) {
                $notes[] = $value_value;
              }
            }
          }
        }
      }
    }
  }
  return $notes;
}

/**
 * Get group machine name.
 */
function spbellen_civi_import_get_group_machine_name($group_id) {
  set_time_limit(0);
  $spcivi = \SPCivi::getInstance();
  $params = array(
    'id' => $group_id,
    'return' => 'name',
  );
  $result = $spcivi->api('Group', 'get', $params);
  if (spbellen_check_result($result, 'een groep', 'get', $params)) {
    if (!empty($result['values'])) {
      $value = array_shift($result['values']);
      $group_name = $value['name'];
      return $group_name;
    }
  }
}
