<?php

/**
 * Helper functions for spbellen civi importer module.
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
      $contact_data = spbellen_proc_import_parse_contact_data($contact_data_raw);
      if ($contact_id = spbellen_import_contact($data['import_id'], $data['group_id'], $contact_data)) {
        $context['results']['count'][$data['group_id']]++;
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
    drupal_set_message('Aantal opgehaalde contacten met geldig telefoonnummer: ' . $total . ' / ' . $total_raw);
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
 * Map civi data to storage contact data.
 */
function spbellen_proc_import_parse_contact_data($proc_data) {
  $contact_data = array();
  $map = array(
    'crm_contact_id' => 'contact_number',
    'name' => 'name',
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
  return $contact_data;
}
