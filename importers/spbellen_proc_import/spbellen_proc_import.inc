<?php

/**
 * Helper functions for spbellen procurios importer module.
 */

/**
 * Test code.
 */
function spbellen_proc_import_dev() {
  //$data = spbellen_proc_import_rest_get_data('https://doemee.lndo.site/rest', 'submission?uuid=eacd6149-93ff-4ab4-a14b-45587c87ba82&page=0&pagesize=10&offset=211121');
  //$data = spbellen_proc_import_rest_get_data('https://doemee.lndo.site/rest', 'webform/eacd6149-93ff-4ab4-a14b-45587c87ba82');
  //$data = spbellen_proc_import_rest_get_data('https://doemee.lndo.site/rest', 'webform');
  //$data = spbellen_proc_import_rest_get_data('https://doemee.lndo.site/rest', 'submission?uuid=eacd6149-93ff-4ab4-a14b-45587c87ba82&page=1&offset=211121');
  //$data = spbellen_proc_import_rest_get_phone_forms('https://doemee.lndo.site/rest');
  return 'Testerdetest';
}

/**
 * Get batch.
 */
function spbellen_proc_import_batch($data, &$context) {
  $context['finished'] = 0;

  if (empty($context['sandbox'])) {
    $context['sandbox']['current_page'] = 1;
    $context['results']['count'][$data['group_id']] = 0;
    $context['results']['count_raw'][$data['group_id']] = 0;
  }

  // Get batch.
  $contact_batch = procapi_selection_get_relations($data['selection_id'], $context['sandbox']['current_page']);

  if (empty($contact_batch)) {
    $context['finished'] = 1;
  }
  else {
    foreach ($contact_batch as $contact_data_raw) {
      $context['results']['count_raw'][$data['group_id']]++;

      // Import contact.
      if ($contact_data = spbellen_proc_import_parse_contact_data($contact_data_raw)) {
        if ($contact_id = spbellen_import_contact($data['import_id'], $data['group_id'], $contact_data)) {
          // Store relation_id contact number mapping.
          if (module_exists('procpush')) {
            if (!empty($contact_data_raw['contact_number']) && !empty($contact_data_raw['relation_id'])) {
              // Remove existing mapping.
              db_delete('procpush_id_map')
                ->condition(
                  db_or()
                  ->condition('contact_number', $contact_data_raw['contact_number'])
                  ->condition('relation_id', $contact_data_raw['relation_id'])
                )
                ->execute();
              db_insert('procpush_id_map')
                ->fields(array(
                  'contact_number' => $contact_data_raw['contact_number'],
                  'relation_id' => $contact_data_raw['id'],
                ))
                ->execute();
            }
          }
          $context['results']['count'][$data['group_id']]++;
        }
        else {
          continue;
        }
      }
      else {
        continue;
      }
    }
    $context['sandbox']['current_page']++;

    // Total number processed.
    $total = 0;
    foreach ($context['results']['count'] as $group_id => $count) {
      $total = $total + $count;
    }
    $total_raw = 0;
    foreach ($context['results']['count_raw'] as $group_id => $count) {
      $total_raw = $total_raw + $count;
    }
    $context['message'] = $total . ' / ' . $total_raw;

    $context['finished'] = round($context['results']['count_raw'][$data['group_id']] / $data['selection_count'], 2, PHP_ROUND_HALF_DOWN);

    // Update import quantity.
    db_update('spbellen_imports')
      ->fields(array(
        'quantity' => $context['results']['count'][$data['group_id']],
      ))
      ->condition('id', $data['import_id'])
      ->execute();
  }
}

/**
 * Finish batch.
 */
function spbellen_proc_import_batch_finished($success, $results, $operations) {
  if ($success) {
    $total = 0;
    foreach ($results['count'] as $group_id => $count) {
      $total = $total + $count;
    }
    $total_raw = 0;
    foreach ($results['count_raw'] as $group_id => $count) {
      $total_raw = $total_raw + $count;
    }
    drupal_set_message('Aantal opgehaalde contacten met geldig telefoonnummer dat gebeld mag worden: ' . $total . ' / ' . $total_raw);
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
 * Map procurios data to storage contact data.
 */
function spbellen_proc_import_parse_contact_data($proc_data) {
  // Check if privacy options are ok for calling.
  if (!empty($proc_data['privacy_options'])) {
    if (isset($proc_data['privacy_options']['do_not_phone'])) {
      if (!empty($proc_data['privacy_options']['do_not_phone'])) {
        return FALSE;
      }
    }
  }
  // Parse contact data.
  $contact_data = array();
  $map = array(
    'crm_contact_id' => 'contact_number',
    'display_name' => 'name',
    'phone' => 'phone_mobile',
    'email' => 'email',
    'street' => 'address|street',
    'street_number' => 'address|number',
    'street_number_addon' => 'address|number_add',
    'postal_code' => 'address|postcode',
    'city' => 'address|town',
    'gender' => 'gender',
    'afdeling_id' => 'department_contact_number',
    'afdeling' => 'department_name',
    'regio_id' => 'region_contact_number',
    'regio' => 'region_name',
  );
  foreach ($map as $data_map => $civi_map) {
    if ($pos = strpos($civi_map, '|') !== FALSE) {
      $elements = explode('|', $civi_map);
      if (!empty($proc_data[$elements[0]][$elements[1]])) {
        $contact_data[$data_map] = $proc_data[$elements[0]][$elements[1]];
      }
    }
    else {
      if (!empty($proc_data[$civi_map])) {
        $contact_data[$data_map] = $proc_data[$civi_map];
      }
    }
  }
  // Check if has mobile phonenumer.
  if (empty($contact_data['phone'])) {
    if (!empty($proc_data['phone'])) {
      $contact_data['phone'] = $proc_data['phone'];
    }
  }
  if (!empty($contact_data['phone'])) {
    $contact_data['phone'] = preg_replace('/\D/', '', $contact_data['phone']);
  }

  // Remove last name .
  if (substr($contact_data['display_name'], -1) == '.') {
    $contact_data['display_name'] = trim(substr_replace($contact_data['display_name'] ,"",-1));
  }
  return $contact_data;
}

/**
 * Autocomplete for selections.
 */
function spbellen_proc_import_selection_autocomplete($string) {
  $string = strtolower($string);
  $matches = array();
  if (strlen($string) > 2) {
    $matches = array();
    $selection_list = procapi_selection_get_select_list();

    foreach ($selection_list as $key => $selection) {
      if (strpos(strtolower($selection), $string) !== FALSE) {
        $matches[$selection . ' (' . $key . ')'] = $selection;
      }
      if (count($matches) > 10) {
        break;
      }
    }
  }

  // Return the result to the form in json.
  drupal_json_output($matches);
}
