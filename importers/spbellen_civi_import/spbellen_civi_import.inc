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
function spbellen_civi_import_get_targets_batch($data, &$context) {
  $context['finished'] = 0;
  // Process by chunks.
  $chunk_size = 20;
  if (empty($context['results']['count'])) {
    $context['results']['count'] = array();
  }

  set_time_limit(0);
  $spcivi = \SPCivi::getInstance();

  if (empty($context['sandbox'])) {

    // Get total number of contacts.
    $number_of_contacts = 0;
    $params = array(
      'group' => $data['civi_group_id'],
      'get_count' => 1,
      'group_contact_id_offset' => $data['last_imported_id'],
    );
    $results = $spcivi->api('Contact', 'getbeltooldata', $params);
    if (spbellen_check_result($results, 'contacten', 'get', $params)) {
      $number_of_contacts = $results['values'][0]['count'];
    }

    $context['sandbox'] = array();
    $context['sandbox']['current_contact'] = 0;
    $context['sandbox']['max'] = $number_of_contacts;
    $context['sandbox']['current_chunk'] = 0;
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['last_imported_id'] = $data['last_imported_id'];

    $context['results']['import_id'] = $data['import_id'];
    $context['results']['external_group_id'] = $data['civi_group_id'];

  }

  $params = array(
    'options' => array('limit' => $chunk_size, 'offset' => $context['sandbox']['progress']),
    'group' => $data['civi_group_id'],
    'group_contact_id_offset' => $data['last_imported_id'],
  );
  $results = $spcivi->api('Contact', 'getbeltooldata', $params);
  if (spbellen_check_result($results, 'contacten', 'get', $params)) {
    if (!empty($results['values'])) {
      foreach ($results['values'] as $key => $result) {
        $crm_contact_id = $result['contact_id'];
        if (isset($result['phone']) || isset($result['mobile'])) {
          // Import contact.
          $contact_data = spbellen_civi_import_map_civi_data($result);
          $contact_id = spbellen_import_contact($data['import_id'], $data['group_id'], $contact_data);
          // Store last imported group contact id.
          $context['sandbox']['last_imported_id'] = $contact_data['group_contact_id'];
          if  (!empty($result['do_not_phone'])) {
            // Blacklist contact.
            spbellen_blacklist($contact_id, FALSE);
          }

          if (empty($context['results']['count'][$data['civi_group_id']])) {
            $context['results']['count'][$data['civi_group_id']] = 0;
          }
          $context['results']['count'][$data['civi_group_id']]++;
        }

        // Update our progress information.
        $context['sandbox']['progress']++;
        $context['sandbox']['current_contact'] = $crm_contact_id;
      }

    }
    else {
      $context['finished'] = 1;
    }
  }

  // Stats.
  $context['sandbox']['current_chunk']++;
  $context['message'] = ($context['sandbox']['current_chunk'] * $chunk_size) . '/' . $context['sandbox']['max'];
  if ($context['finished'] !== 1) {
    $context['finished'] = ($context['sandbox']['progress'] / $context['sandbox']['max']);
  }

  // Finish batch.
  if ($context['finished'] === 1) {
    // Store import total.
    db_merge('spbellen_groups')
      ->key(array(
        'id' => $data['group_id'],
      ))
      ->fields(array(
        'external_last_imported_id' => $context['sandbox']['last_imported_id'],
      ))
      ->execute();
  }
}

/**
 * Finish batch.
 */
function spbellen_civi_import_get_targets_batch_finished($success, $results, $operations) {
  if ($success) {
    $total = array_sum($results['count']);
    drupal_set_message('Aantal opgehaalde contacten: ' . $total);
    // Store import total.
    db_merge('spbellen_imports')
      ->key(array(
        'id' => $results['import_id'],
      ))
      ->fields(array(
        'quantity' => $total,
      ))
      ->execute();
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

/**
 * Create CiviCRM activity.
 */
function spbellen_civi_import_create_activity($crm_contact_id, $activity_type, $status, $subject, $text, $debug_info) {
  $spcivi = \SPCivi::getInstance();
  set_time_limit(60);
  $params = array(
    'activity_type_id' => $activity_type,
    'subject' => $subject,
    'target_id' => $crm_contact_id,
    'details' => $text,
    'status_id' => $status,
    'priority_id' => 2,
  );
  $activity_results = $spcivi->api('Activity', 'create', $params);
}

/**
 * Map civi data to storage contact data.
 */
function spbellen_civi_import_map_civi_data($civi_data) {
  $map = array(
    'crm_contact_id' => 'contact_id',
    'phone' => 'phone',
    'display_name' => 'display_name',
    'email' => 'email',
    'street_address' => 'street_address',
    'postal_code' => 'postal_code',
    'city' => 'city',
    'gender' => 'gender',
    'afdeling_id' => 'afdeling_code',
    'afdeling' => 'afdeling',
    'regio_id' => 'regio_code',
    'regio' => 'regio',
    'group_contact_id' => 'group_contact_id',
  );
  foreach ($map as $data_map => $civi_map) {
    if (isset($civi_data[$civi_map])) {
      $contact_data[$data_map] = $civi_data[$civi_map];
    }
  }
  // Check if has mobile phonenumer.
  if (!empty($civi_data['mobile'])) {
    $contact_data['phone'] = $civi_data['mobile'];
  }
  if (!empty($contact_data['phone'])) {
    $contact_data['phone'] = preg_replace('/\D/', '', $contact_data['phone']);
  }
  // Get notes.
  $notes = spbellen_civi_import_get_notes($civi_data['contact_id']);
  if (!empty($notes)) {
    $notes_serialized = serialize($notes);
    $contact_data['notes'] = $notes_serialized;
  }
  return $contact_data;
}
