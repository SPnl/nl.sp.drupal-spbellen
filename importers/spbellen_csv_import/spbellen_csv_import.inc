<?php

/**
 * Helper functions for spbellen csv importer module.
 */

/**
 * Detect delimiter in file.
 */
function spbellen_csv_import_detect_delimiter($file) {
  $realpath = drupal_realpath($file->uri);
  $found = FALSE;
  $file_object = new SplFileObject($realpath);
  $delimiters = ["\t", ";", "|", ","];
  $data_1 = NULL; $data_2 = NULL;
  $delimiter = $delimiters[0];
  foreach ($delimiters as $d) {
    $data_1 = $file_object->fgetcsv($d);
    if (count($data_1) > 1) {
      if (count($data_1) > count($data_2)) {
        $found = TRUE;
        $delimiter = count($data_1) > count($data_2) ? $d : $delimiter;
        $data_2 = $data_1;
      }
      $file_object->rewind();
    }
  }
  if ($found) {
    return $delimiter;
  }
  return FALSE;
}

/**
 * Parse csv field mapping data.
 */
function spbellen_csv_import_parse_csv_field_mapping_data($file, $delimiter) {
  $data = array();
  $realpath = drupal_realpath($file->uri);
  if (($fh = fopen($realpath, 'r'))) {
    for ($row_number = 0; $row_number < 5; $row_number++) {
      $row = fgetcsv($fh, 4096, $delimiter);
      if ($row) {
        foreach ($row as $column_number => $value) {
          $data[$column_number][$row_number] = '&bull;&nbsp;' . $value;
        }
      }
    }
    return $data;
  }
  return FALSE;
}

/**
 * Process batch.
 */
function spbellen_csv_import_batch($storage, &$context) {
  // Process by chunks.
  $chunk_size = 50;

  $file = $storage['upload'];
  $realpath = drupal_realpath($file->uri);
  $file_object = new SplFileObject($realpath);
  $file_object->setFlags(SplFileObject::READ_CSV);
  $file_object->setCsvControl($storage['delimiter']);

  $context['results']['import_id'] = $storage['import_id'];

  if (empty($context['sandbox'])) {
    // Get number of rows to import.
    $file_object->seek(PHP_INT_MAX);
    $rowcount = $file_object->key();
    $file_object->rewind();

    $context['sandbox'] = array();
    $context['sandbox']['progress'] = $storage['skip_first_row'];
    $context['sandbox']['current_row'] = 0;
    $context['sandbox']['max'] = $rowcount;
    $context['sandbox']['current_chunk'] = 0;
    $context['results']['count'] = 0;
  }

  // Get rows.
  $rows = array();
  $file_object->seek($context['sandbox']['progress']);
  for ($i = 0; $i < $chunk_size; $i++) {
    $file_object->seek($context['sandbox']['progress']);
    $row = $file_object->current();
    if ($file_object->eof()) {
      $context['finished'] = 1;
      return;
    }
    $result = spbellen_csv_import_batch_process_row($context['sandbox']['progress'], $row, $storage);
    if ($result) {
      $context['results']['count']++;
    }
    // Update our progress information.
    $context['sandbox']['progress']++;
  }

  $context['sandbox']['current_chunk']++;
  $context['message'] = ($context['sandbox']['current_chunk'] * $chunk_size) . '/' . $context['sandbox']['max'];
  $context['finished'] = ($context['sandbox']['progress'] / $context['sandbox']['max']);
}

/**
 * Batch process row.
 */
function spbellen_csv_import_batch_process_row($row_number, $row, $storage) {
  global $user;

  $count = 0;
  $contact_data = array();
  foreach ($storage['csv_beltool_mapping'] as $csv_field_nr => $selected_beltool_field) {
    $count++;
    $value = spbellen_csv_import_csvfieldtrim($row[$csv_field_nr]);

    // Clean phone number.
    if ($selected_beltool_field === 'phone') {
      $value = preg_replace('@\D@', '', $value);
    }

    // Check if component is required, and has value, else, skip row.
    if ($value == '') {
      if (in_array($selected_beltool_field, array(
        'phone',
      ))) {
        drupal_set_message('Verplichte veldwaarde (' . $storage['contact_fields'][$selected_beltool_field] . ') ontbreekt, rij (nr. ' . $row_number . ') wordt overgeslagen.', 'warning');
        return FALSE;
      }
      else {
        continue;
      }
    }
    $contact_data[$storage['csv_beltool_mapping'][$csv_field_nr]] = $value;
  }

  // Store contact.
  $contact_id = spbellen_import_contact($storage['campaign'], $storage['import_id'], $contact_data);

  return TRUE;
}

/**
 * Batch finished.
 */
function spbellen_csv_import_batch_finished($success, $results, $operations) {
  if ($success) {
    $total = $results['count'];
    drupal_set_message('Aantal opgehaalde contacten: ' . $results['count']);
    db_update('spbellen_import_history')
      ->fields(array(
        'quantity' => $total,
      ))
      ->condition('id', $results['import_id'])
      ->execute();
  }
}

/**
 * Trim csv field value.
 */
function spbellen_csv_import_csvfieldtrim($value) {
  $value = trim($value);
  // Strip off the beginning and ending quotes if necessary.
  $value = trim($value, '"');
  return $value;
}
