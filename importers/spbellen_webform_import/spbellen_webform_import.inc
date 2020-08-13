<?php

/**
 * Helper functions for spbellen civi importer module.
 */

/**
 * Test code.
 */
function spbellen_webform_import_dev() {
  //$data = spbellen_webform_import_rest_get_data('https://doemee.lndo.site/rest', 'submission?uuid=eacd6149-93ff-4ab4-a14b-45587c87ba82&page=0&pagesize=10&offset=211121');
  //$data = spbellen_webform_import_rest_get_data('https://doemee.lndo.site/rest', 'webform/eacd6149-93ff-4ab4-a14b-45587c87ba82');
  //$data = spbellen_webform_import_rest_get_data('https://doemee.lndo.site/rest', 'webform');
  //$data = spbellen_webform_import_rest_get_submissions('https://doemee.lndo.site/rest', 'eacd6149-93ff-4ab4-a14b-45587c87ba82', '211121', 2);
  $data = spbellen_webform_import_rest_get_data('https://doemee.lndo.site/rest', 'submission?uuid=eacd6149-93ff-4ab4-a14b-45587c87ba82&page=1&offset=211121');
  return 'Testerdetest';
}

/**
 * Get submissions.
 */
function spbellen_webform_import_rest_get_submissions($rest_endpoint, $webform_uuid, $last_imported_id, $batch_nr, $chunk_size) {
  $request = 'submission?uuid=' . $webform_uuid . '&page=' . $batch_nr . '&pagesize=' . $chunk_size . '&offset=' . $last_imported_id;
  $data = spbellen_webform_import_rest_get_data($rest_endpoint, $request);
  return $data;
}

/**
 * Get submission total count.
 */
function spbellen_webform_import_rest_get_submissions_count($rest_endpoint, $webform_uuid, $last_imported_id) {
  $request = 'submission?uuid=' . $webform_uuid . '&offset=' . $last_imported_id . '&count=1';
  $data = spbellen_webform_import_rest_get_data($rest_endpoint, $request);
  if (isset($data[0])) {
    return $data[0];
  }
  return FALSE;
}

/**
 * Get phone forms via REST.
 */
function spbellen_webform_import_rest_get_phone_forms($rest_endpoint) {
  $webforms = spbellen_webform_import_rest_get_webforms($rest_endpoint);
  $phone_forms = spbellen_webform_import_filter_phone_forms($webforms);
  return $phone_forms;
}

/**
 * Filter out forms with phone contact information fields.
 */
function spbellen_webform_import_filter_phone_forms($webforms) {
  $contact_forms = array();
  foreach ($webforms as $webform) {
    $name_found = FALSE;
    $phone_found = FALSE;
    if (!empty($webform['webform']['components'])) {
      foreach ($webform['webform']['components'] as $component) {
        switch ($component['type']) {
          case 'spcontact':
            $phone_found = TRUE;
            $name_found = TRUE;
            break;

          case 'phone':
            $phone_found = TRUE;
            break;

          case 'textfield':
            if (in_array($component['form_key'], array(
              'naam',
              'name',
              'voornaam',
              'first_name',
              'achternaam',
              'last_name',
            ))) {
              $name_found = TRUE;
            }
            if (in_array($component['form_key'], array(
              'telefoon',
              'telefoonnummer',
              'phone',
              'phone_number',
            ))) {
              $phone_found = TRUE;
            }
            break;
        }
        if ($phone_found && $name_found) {
          break;
        }
      }
    }
    if ($phone_found && $name_found) {
      $contact_forms[$webform['uuid']] = $webform;
    }
  }
  return $contact_forms;
}

/**
 * Get remote webforms.
 */
function spbellen_webform_import_rest_get_webforms($rest_endpoint) {
  $webforms = array();
  $types = variable_get('spbellen_webform_import_node_types');

  foreach ($types as $type) {
    for ($i = 0; $i < 50; $i++) {

      $request = $type . '?page=' . $i;

      $data = spbellen_webform_import_rest_get_data($rest_endpoint, $request);

      if (empty($data)) {
        break;
      }

      foreach ($data as $webform) {
        $webforms[] = $webform;
      }
    }
  }

  return $webforms;
}

/**
 * Get REST data.
 */
function spbellen_webform_import_rest_get_data($rest_endpoint, $request) {
  $creds = spbellen_webform_import_rest_get_credentials($rest_endpoint);

  if (empty($creds['token']) || empty($creds['session_id']) || empty($creds['rest_endpoint'])) {
    return FALSE;
  }

  $data = array();

  $ch = curl_init();

  // Define headers.
  $headers = array(
    'Content-Type: application/json',
    'X-Csrf-Token: ' . $creds['token'],
    'Cookie: ' . $creds['session_id'],
  );

  $url = $creds['rest_endpoint'] . '/' . $request;

  $options = array(
    CURLOPT_URL => $url,
    CURLOPT_RETURNTRANSFER => 1,
    CURLOPT_HTTPHEADER => $headers,
    //CURLOPT_HEADER => 1,
  );

  curl_setopt_array($ch, $options);
  $result_json = curl_exec($ch);
  curl_close($ch);

  if (empty($result_json)) {
    watchdog('spbellen_webform_import', 'Er is een fout opgetreden bij het ophalen van webforms (curl).<pre>' . print_r($options, TRUE) . '</pre>');
    return FALSE;
  }

  $data = json_decode($result_json, TRUE);

  if (is_null($data)) {
    watchdog('spbellen_webform_import', 'Er is een fout opgetreden bij het ophalen van webforms (json_decode).');
    return FALSE;
  }

  return $data;
}

/**
 * Get webform site session id.
 */
function spbellen_webform_import_rest_get_credentials($rest_endpoint) {
  // Create curl resource.
  // Step 1: get token.
  $ch = curl_init();

  // Define headers.
  $headers = array(
    'Content-Type: application/json',
  );

  $options = array(
    CURLOPT_URL => $rest_endpoint . '/user/token.json',
    CURLOPT_POST => 1,
    CURLOPT_RETURNTRANSFER => 1,
    CURLOPT_HTTPHEADER => $headers,
  );

  curl_setopt_array($ch, $options);
  $result_json = curl_exec($ch);
  curl_close($ch);

  if (empty($result_json)) {
    watchdog('spbellen_webform_import', 'Er is een fout opgetreden bij het ophalen van een rest token (curl).<pre>' . print_r($options, TRUE) . '</pre>');
    return FALSE;
  }

  $result = json_decode($result_json, TRUE);
  if (is_null($result)) {
    watchdog('spbellen_webform_import', 'Er is een fout opgetreden bij het ophalen van een rest token (json_decode).');
    return FALSE;
  }

  if (empty($result['token'])) {
    watchdog('spbellen_webform_import', 'Er is een fout opgetreden bij het ophalen van een rest token, geen token in respons.');
    return FALSE;
  }

  $token = $result['token'];

  // Step 2: Authenticate.
  $ch = curl_init();

  // Define headers.
  $headers = array(
    'Content-Type: application/json',
    'X-Csrf-Token: ' . $token,
  );

  // Define body.
  $creds = variable_get('spbellen_webform_import_creds');
  $body = json_encode($creds);

  $options = array(
    CURLOPT_URL => $rest_endpoint . '/user/login.json',
    CURLOPT_POST => 1,
    CURLOPT_RETURNTRANSFER => 1,
    CURLOPT_HTTPHEADER => $headers,
    CURLOPT_POSTFIELDS => $body,
  );

  curl_setopt_array($ch, $options);
  $result_json = curl_exec($ch);
  curl_close($ch);

  if (empty($result_json)) {
    watchdog('spbellen_webform_import', 'Er is een fout opgetreden bij het authenticeren voor de rest verbinding (curl).<pre>' . print_r($options, TRUE) . '</pre>');
    return FALSE;
  }

  $result = json_decode($result_json, TRUE);

  if (is_null($result)) {
    watchdog('spbellen_webform_import', 'Er is een fout opgetreden bij het authenticeren voor de rest verbinding (json_decode).');
    return FALSE;
  }

  if (empty($result['sessid'])) {
    watchdog('spbellen_webform_import', 'Er is een fout opgetreden bij het ophalen van de session id.');
    return FALSE;
  }

  $creds = array(
    'token' => $token,
    'session_id' => $result['sessid'],
    'rest_endpoint' => $rest_endpoint,
  );

  return $creds;
}

/**
 * Get batch.
 */
function spbellen_webform_import_batch($data, &$context) {
  $context['finished'] = 0;

  if (empty($context['sandbox'])) {
    $context['sandbox']['current_chunk'] = 0;
    $context['sandbox']['count'] = 0;
  }

  // Get batch.
  $submissions = spbellen_webform_import_rest_get_submissions($data['rest_endpoint'], $data['webform_uuid'], $data['last_imported_id'], $context['sandbox']['current_chunk'], $data['chunk_size']);
  if (!empty($submissions)) {
    foreach ($submissions as $submission) {
      // Check if submission has selected options.
      if (!spbellen_webform_import_select_option_filter_pass($submission, $data['select_options'])) {
        continue;
      }
      // Import contact.
      $contact_data = spbellen_webform_import_parse_contact_data($submission);

      if (!empty($contact_data)) {
        $contact_id = spbellen_import_contact($data['import_id'], $data['group_id'], $contact_data);
        if ($contact_id === FALSE) {
          continue;
        }
      }

      $context['results']['count']++;
    }

    // Update our progress information.
    $context['sandbox']['current_chunk']++;
    $context['finished'] = ($context['sandbox']['current_chunk'] / $data['chunks']);
    $context['message'] = ($context['sandbox']['current_chunk'] * $chunk_size) . '/' . $data['$total_number_of_submissions'];

    // Update import quantity.
    db_update('spbellen_imports')
      ->fields(array(
        'quantity' => $context['results']['count'],
      ))
      ->condition('id', $data['import_id'])
      ->execute();
    // Update last imported id.
    db_update('spbellen_groups')
      ->fields(array(
        'external_last_imported_id' => $submission['sid'],
      ))
      ->condition('id', $data['group_id'])
      ->execute();
  }
  else {
    $context['finished'] = 1;
  }
}

/**
 * Finish batch.
 */
function spbellen_webform_import_batch_finished($success, $results, $operations) {
  if ($success) {
    $total = $results['count'];
    drupal_set_message('Aantal opgehaalde contacten: ' . $total);
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
function spbellen_webform_import_get_notes($crm_contact_id) {
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
function spbellen_webform_import_get_group_machine_name($group_id) {
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
function spbellen_webform_import_create_activity($crm_contact_id, $activity_type, $status, $subject, $text, $debug_info) {
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
function spbellen_webform_import_map_civi_data($civi_data) {
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
  $notes = spbellen_webform_import_get_notes($civi_data['contact_id']);
  if (!empty($notes)) {
    $notes_serialized = serialize($notes);
    $contact_data['notes'] = $notes_serialized;
  }
  return $contact_data;
}

function spbellen_webform_import_parse_webform_select_options($component) {
  $options = array();
  switch ($component['type']) {
    case 'select':
      if (!empty($component['extra']['items'])) {
        $items = $component['extra']['items'];
        $lines = explode("\n", trim($items));
        foreach ($lines as $line) {
          $line = trim($line);
          $matches = array();
          if (preg_match('/^([^|]*)\|(.*)$/', $line, $matches)) {
            $options[$matches[1]] = empty($matches[2]) ? $matches[1] : $matches[2];
          }
        }
      }
      break;

    case 'spcontact':
      if (!empty($component['extra']['optional_questions'])) {
        $items = $component['extra']['optional_questions'];
        foreach ($items as $item) {
          if (!empty($item)) {
            $spcontact_items = spbellen_webform_import_spcontact_select_options($component);
            $options[$item] = $spcontact_items[$item];
          }
        }
      }
      break;
  }
  return $options;
}

/**
 * Default select options for spcontact.
 */
function spbellen_webform_import_spcontact_default_select_options() {
  return array(
    'keep_informed' => 'Hou me op de hoogte van deze actie',
    'sp_news' => 'Hou me op de hoogte van belangrijk SP nieuws',
    'send_something' => 'Stuur me iets op',
    'action' => 'Ik wil iets doen',
    'membership' => 'Ik wil lid worden',
  );
}

/**
 * Customized select options texts for spcontact.
 */
function spbellen_webform_import_spcontact_select_options($component, $original_texts = FALSE) {
  $options = spbellen_webform_import_spcontact_default_select_options();
  if (!$original_texts) {
    foreach ($options as $key => $question) {
      if (!empty($component['extra'][$key . '_custom_text'])) {
        $options[$key] = $component['extra'][$key . '_custom_text'];
      }
    }
  }
  return $options;
}
