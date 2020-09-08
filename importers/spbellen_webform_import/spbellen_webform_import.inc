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
  //$data = spbellen_webform_import_rest_get_data('https://doemee.lndo.site/rest', 'submission?uuid=eacd6149-93ff-4ab4-a14b-45587c87ba82&page=1&offset=211121');
  //$data = spbellen_webform_import_rest_get_phone_forms('https://doemee.lndo.site/rest');
  //dpm($data);
  $data = spbellen_webform_import_rest_get_submissions('https://doemee.lndo.site/rest', '78f3ddf2-2f0a-4dde-8fe2-8dcd94b9b2ff', 0, 2, 20);
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
  $submissions = spbellen_webform_import_rest_get_submissions($data['rest_endpoint'], $data['webform']['uuid'], $data['last_imported_id'], $context['sandbox']['current_chunk'], $data['chunk_size']);
  if (!empty($submissions)) {
    foreach ($submissions as $submission) {
      // Check if submission has required options.
      // Import contact.
      $contact_data = spbellen_webform_import_parse_contact_data($submission, $data['required_options']);

      if (empty($contact_data)) {
        continue;
      }

      $contact_id = spbellen_import_contact($data['import_id'], $data['group_id'], $contact_data);
      if ($contact_id === FALSE) {
        continue;
      }

      // Store submission.
      $submission_json = json_encode($submission);
      if ($submission_json !== FALSE) {
        db_merge('spbellen_webform_import_submission_data')
          ->key(array(
            'group_id' => $data['group_id'],
            'contact_id' => $contact_id,
          ))
          ->fields(array(
            'submission' => $submission_json,
          ))
          ->execute();
      }

      $context['results']['count']++;
    }

    // Update our progress information.
    $context['sandbox']['current_chunk']++;
    $context['finished'] = ($context['sandbox']['current_chunk'] / $data['chunks']);
    $context['message'] = ($context['sandbox']['current_chunk'] * $data['chunk_size']) . '/' . $data['total_number_of_submissions'];

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

function spbellen_webform_import_spcontact_component_option_id_mapping() {
  return array(
    'keep_informed' => 7,
    'spnews' => 8,
    'send_something' => 9,
    'action' => 10,
    'membership' => 11,
  );
}

function spbellen_webform_import_spcontact_component_contact_data_mapping() {
  return array(
    'display_name' => 0,
    'email' => 1,
    'phone' => 2,
    'postal_code' => 3,
    'house_number' => 4,
    'street_address' => 5,
    'city' => 6,
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

/**
 * Check if required options for this import are met by the selected values in the submission.
 */
function spbellen_webform_import_parse_contact_data($submission, $required_options) {
  $contact_data = array();

  $phone_found = FALSE;
  $name_found = FALSE;

  foreach ($submission['data'] as $respons) {
    switch ($respons['type']) {
      case 'select':
        // Check if required options are set.
        if (!empty($required_options[$respons['cid']])) {
          foreach ($required_options[$respons['cid']] as $required_option) {
            if (!empty($required_option)) {
              if (array_search($required_option, $respons['values']) === FALSE) {
                return FALSE;
              }
            }
          }
        }
        break;

      case 'spcontact':
        $option_mapping = spbellen_webform_import_spcontact_component_option_id_mapping();
        $data_mapping = spbellen_webform_import_spcontact_component_contact_data_mapping();
        // Check if required options are selected.
        if (!empty($required_options[$respons['cid']])) {
          foreach ($required_options[$respons['cid']] as $required_option) {
            if (!empty($required_option)) {
              if (empty($respons['values'][$option_mapping[$required_option]])) {
                return FALSE;
              }
            }
          }
        }
        // Set contact data.
        if (!empty($respons['values'][$data_mapping['display_name']])) {
          $contact_data['display_name'] = $respons['values'][$data_mapping['display_name']];
        }
        if (!empty($respons['values'][$data_mapping['email']])) {
          $contact_data['email'] = $respons['values'][$data_mapping['email']];
        }
        if (!empty($respons['values'][$data_mapping['phone']])) {
          $contact_data['phone'] = $respons['values'][$data_mapping['phone']];
        }
        if (!empty($respons['values'][$data_mapping['postal_code']])) {
          $contact_data['postal_code'] = $respons['values'][$data_mapping['postal_code']];
        }
        if (!empty($respons['values'][$data_mapping['street_address']])) {
          $contact_data['street_address'] = $respons['values'][$data_mapping['street_address']];
          if (!empty($respons['values'][$data_mapping['house_number']])) {
            $contact_data['street_address'] .= ' ' . $respons['values'][$data_mapping['house_number']];
          }
        }
        if (!empty($respons['values'][$data_mapping['city']])) {
          $contact_data['city'] = $respons['values'][$data_mapping['city']];
        }
        break;

      case 'textfield':
        if (in_array($respons['form_key'], array(
          'naam',
          'name',
          'voornaam',
          'first_name',
          'achternaam',
          'last_name',
        ))) {
          if (!empty($respons['values'][0])) {
            $contact_data['display_name'] = $respons['values'][0];
          }
        }
        if (in_array($respons['form_key'], array(
          'telefoon',
          'telefoonnummer',
          'phone',
          'phone_number',
        ))) {
          if (!empty($respons['values'][0])) {
            $contact_data['phone'] = $respons['values'][0];
          }
        }
        if (in_array($respons['form_key'], array(
          'postcode',
          'postal_code',
        ))) {
          if (!empty($respons['values'][0])) {
            $contact_data['postal_code'] = $respons['values'][0];
          }
        }
        break;

      case 'phone':
        if (!empty($respons['values'][0])) {
          $contact_data['phone'] = $respons['values'][0];
        }
        break;

      case 'sp_address_auto':
        if (!empty($respons['values']['postal_code'])) {
          $contact_data['postal_code'] = $respons['values']['postal_code'];
        }
        if (!empty($respons['values']['city'])) {
          $contact_data['city'] = $respons['values']['city'];
        }
        if (!empty($respons['values']['street_name'])) {
          $contact_data['street_address'] = $respons['values']['street_name'];
          if (!empty($respons['values']['house_number'])) {
            $contact_data['street_address'] .= ' ' . $respons['values']['house_number'];
          }
        }
        break;

      case 'email':
        if (!empty($respons['values'][0])) {
          $contact_data['email'] = $respons['values'][0];
        }
        break;
    }
  }
  // Check if name and phone are set.
  if (empty($contact_data['display_name']) || empty($contact_data['phone'])) {
    return FALSE;
  }
  return $contact_data;
}
