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
