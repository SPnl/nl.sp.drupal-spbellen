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
function spbellen_civi_import_get_targets_batch($import_info, $group_info, &$context) {
  // Process by chunks.
  $chunk_size = 20;
  if (empty($context['results']['count'])) {
    $context['results']['count'] = array();
  }

  set_time_limit(0);
  $spcivi = \SPCivi::getInstance();

  $context['results']['import_id'] = $import_info['import_id'];

  if (empty($context['sandbox'])) {
    // Get total number of contacts.
    $number_of_contacts = 0;
    $params = array(
      'group' => $group_info['id'],
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
    'group' => $group_info['id'],
    'include_non_menmbers' => 1,
  );
  $results = $spcivi->api('Contact', 'getspdata', $params);
  if (spbellen_check_result($results, 'contacten', 'get', $params)) {
    if (!empty($results['values'])) {
      foreach ($results['values'] as $key => $result) {
        $crm_contact_id = $result['contact_id'];
        if ((isset($result['phone']) || isset($result['mobile'])) && empty($result['do_not_phone'])) {
          // Process contact.
          $contact_data = spbellen_civi_import_map_civi_data($result);
          $contact_id = spbellen_import_contact($import_info['campaign_id'], $import_info['id'], $contact_data);

          if (empty($context['results']['count'][$group_id])) {
            $context['results']['count'][$group_id] = 0;
          }
          $context['results']['count'][$group_id]++;
        }

        // Update our progress information.
        $context['sandbox']['progress']++;
        $context['sandbox']['current_contact'] = $crm_contact_id;
      }

    }
    else {
      $context['finished'] = 1;
      return;
    }
  }

  // Stats.
  $context['sandbox']['current_chunk']++;
  $context['message'] = ($context['sandbox']['current_chunk'] * $chunk_size) . '/' . $context['sandbox']['max'];
  $context['finished'] = ($context['sandbox']['progress'] / $context['sandbox']['max']);
}

/**
 * Finish batch.
 */
function spbellen_civi_import_get_targets_batch_finished($success, $results, $operations) {
  if ($success) {
    $total = array_sum($results['count']);
    drupal_set_message('Aantal opgehaalde contacten: ' . $total);
    // Store import total.
    db_update('spbellen_import_history')
      ->fields(array(
        'quantity' => $total,
      ))
      ->condition('id', $results['import_id'])
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
  $contact_data['crm_contact_id'] = $civi_data['contact_id'];
  $contact_data['phone'] = $civi_data['phone'];
  // Check if has mobile phonenumer.
  if (!empty($civi_data['mobile'])) {
    $contact_data['phone'] = $civi_data['mobile'];
  }
  $contact_data['phone'] = preg_replace('/\D/', '', $contact_data['phone']);
  $contact_data['display_name'] = $civi_data['display_name'];
  $contact_data['email'] = $civi_data['email'];
  $contact_data['street_address'] = $civi_data['street_address'];
  $contact_data['postal_code'] = $civi_data['postal_code'];
  $contact_data['city'] = $civi_data['city'];
  $contact_data['gender'] = $civi_data['gender'];
  $contact_data['is_lid'] = $civi_data['is_member'];
  $contact_data['afdeling_id'] = empty($civi_data['afdeling_code']) ? '' : $civi_data['afdeling_code'];
  $contact_data['afdeling'] = $civi_data['afdeling'];
  // Get notes.
  $notes = spbellen_civi_import_get_notes($civi_data['contact_id']);
  if (!empty($notes)) {
    $notes_serialized = serialize($notes);
    $contact_data['notes'] = $notes_serialized;
  }
  return $contact_data;
}
