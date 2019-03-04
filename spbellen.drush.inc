<?php

/**
 * @file
 * Implements drush commands.
 */

/**
 * Implements hook_drush_command().
 */
function spbellen_drush_command() {
  $items = array();
  $items['merge_duplicates'] = array(
    'callback' => 'spbellen_merge_duplicates',
    'description' => 'Drush command to merge duplicates in contact data.',
    'aliases' => array('spbmd'),
    'examples' => array(
      'Merge duplicates' => 'drush spbellen_merge_duplicates',
      'Merge duplicates' => 'drush spbmd',
    ),
  );
  $items['remove_fake_phone_numbers'] = array(
    'callback' => 'spbellen_remove_fake_phone_numbers',
    'description' => 'Drush command to remove fake phone numbers.',
    'aliases' => array('spbrf'),
    'examples' => array(
      'Remove fake phone numbers' => 'drush spbellen_remove_fake_phone_numbers',
      'Remove fake phone numbers' => 'drush spbrf',
    ),
  );
  return $items;
}

/**
 * Remove fake phone numbers.
 */
function spbellen_remove_fake_phone_numbers() {
  // Cleanup false phone numbers.
  drush_print('Removing fake phone numbers with repetetive digits.');
  $regexp = '0{6}|1{6}2{6}|3{6}|4{6}|5{6}|6{6}|7{6}|8{6}|9{6}';
  $query = "DELETE cd, ta, ac FROM spbellen_contact_data cd LEFT JOIN spbellen_targets ta ON cd.contact_id = ta.contact_id LEFT JOIN spbellen_action ac ON ac.target_id = ta.id WHERE phone REGEXP :regexp";
  db_query($query, array('regexp' => $regexp));
  drush_print('Removing fake phone numbers with repetetive alternating digits.');
  for ($i = 0; $i < 10; $i++) {
    for ($j = 0; $j < 10; $j++) {
      $pattern1 = '';
      $pattern2 = '';
      for ($k = 0; $k < 4; $k++) {
        $pattern1 .= $i . $j;
        $pattern2 .= $i . $j;
      }
      $query = "DELETE cd, ta, ac FROM {spbellen_contact_data} cd LEFT JOIN {spbellen_targets} ta ON cd.contact_id = ta.contact_id LEFT JOIN {spbellen_action} ac ON ac.target_id = ta.id WHERE phone LIKE :pattern";
      $result = db_query($query, array('pattern' => '%' . db_like($pattern1) . '%'));
      $query = "DELETE cd, ta, ac FROM {spbellen_contact_data} cd LEFT JOIN {spbellen_targets} ta ON cd.contact_id = ta.contact_id LEFT JOIN {spbellen_action} ac ON ac.target_id = ta.id WHERE phone LIKE :pattern";
      $result = db_query($query, array('pattern' => '%' . db_like($pattern2) . '%'));
    }
  }
}

/**
 * Callback for hook_drush_command.
 */
function spbellen_merge_duplicates() {
  // Get duplicate phone numbers.
  drush_print('Merging duplicate contacts.');
  $query = "SELECT da1.phone phone FROM {spbellen_contact_data} da1 INNER JOIN {spbellen_contact_data} da2 ON da1.contact_id != da2.contact_id AND da1.phone = da2.phone AND da2.crm_contact_id IS NULL LIMIT 100;";
  $result = db_query($query);
  if ($result) {
    while ($row = $result->fetchAssoc()) {
      if (!empty($row['phone'])) {
        spbellen_contact_merge($row['phone']);
      }
    }
  }
}

/**
 * Merge contact data from contacts with same phone number.
 */
function spbellen_contact_merge($phone) {
  if (!empty($phone)) {
    drush_print('Phone: ' . $phone);
    $query = "SELECT * FROM {spbellen_contact_data} WHERE phone = :phone";
    $result = db_query($query, array(':phone' => $phone));
    if ($result) {
      $data = array();
      while ($row = $result->fetchAssoc()) {
        $data[$row['contact_id']] = $row;
      }
      // Find same contacts.
      $same_contacts = array();
      $same_contacts_final = array();
      foreach ($data as $contact_id => $contact_data) {
        // 06 number.
        if (substr($contact_data['phone'], 0, 2) === '06') {
          $same_contacts['mobile'][$contact_id] = $contact_id;
        }
        elseif (!empty($contact_data['display_name'])) {
          // Name.
          $name_parts = spbellen_split_name($contact_data['display_name']);
          if (!empty($name_parts['first_name'])) {
            $same_contacts['name'][$name_parts['first_name']][$contact_id] = $contact_id;
          }
          else {
            $same_contacts['name'][$contact_data['display_name']][$contact_id] = $contact_id;
          }
        }
      }
      if (!empty($same_contacts['name'])) {
        foreach ($same_contacts['name'] as $name => $contact_ids) {
          if (count($contact_ids) > 1) {
            foreach ($contact_ids as $contact_id) {
              $same_contacts_final[$contact_id] = $contact_id;
            }
          }
        }
      }
      if (!empty($same_contacts['mobile'])) {
        if (count($same_contacts['mobile']) > 1) {
          foreach ($same_contacts['mobile'] as $contact_id) {
            $same_contacts_final[$contact_id] = $contact_id;
          }
        }
      }
      if (!empty($same_contacts_final)) {
        drush_print('Same contacts:');
        $output = drush_format($same_contacts_final);
        drush_print($output);
        // Merge same contacts.
        spbellen_merge_contacts($same_contacts_final);
      }
    }
  }
}

/**
 * Split name.
 */
function spbellen_split_name($name) {
  $matches = array();
  $name = trim($name);
  $name = trim($name, '.,');

  // Remove comma from name.
  $comma_pos = strpos($name, ',');
  if ($comma_pos !== FALSE) {
    $first_half = trim(substr($name, 0, $comma_pos));
    $last_half = trim(substr($name, $comma_pos + 1));
    $name = $last_half . ' ' . $first_half;
  }

  // Replace terms.
  $replace = array(
    'v' => 'van',
    'v.' => 'van',
    'vd' => 'van de',
    'vd.' => 'van de',
    'v.d.' => 'van de',
    'v/d' => 'van de',
  );
  foreach ($replace as $orig => $new) {
    if (strpos($name, ' ' . $orig . ' ') !== FALSE) {
      $name = str_replace($orig, $new, $name);
    }
  }

  // Regexes for title and middle names.
  $titles_regex = 'Mr|mr|Mevr|mevr|Dhr|dhr|Ir|ir|Ing|ing|Bc|bc|Ms|ms|Drs|drs|Dr|dr';

  $middle_names_array = spcivipush_middle_names();
  $middle_names_regex = implode('|', $middle_names_array);

  // Split name on middle_name.
  $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'first_name'(([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-\.]+)\s*)+?)(?'middle_name'(\b(" . $middle_names_regex . ")\b\s*))(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][a-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+)$@";
  $result = preg_match($regex, $name, $matches);
  if (!preg_match($regex, $name, $matches)) {
    // Split name on middle name, last name first.
    $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+),?\s(?'first_name'(([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-\.]+)\s*)+?)(?'middle_name'(\b(" . $middle_names_regex . ")\b\s*))$@";
    $result = preg_match($regex, $name, $matches);
    if (!preg_match($regex, $name, $matches)) {
      // Split name without middle_name in first name and last name(s).
      $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'first_name'(([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-\.]+)\s*)+?)(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+)$@";
      $result = preg_match($regex, $name, $matches);
      if (!preg_match($regex, $name, $matches)) {
        // Split name in first name and last name, last name first.
        $regex = "@^(?'title'(\b(" . $titles_regex . ")\b\.?\s)?)(?'last_name'(\b[A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+\s?)+),?\s+(?'first_name'((([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ']\.)+)\s*)+?)$@";
        $result = preg_match($regex, $name, $matches);
        if (!preg_match($regex, $name, $matches)) {
          // Check if only first name.
          $regex = "@^(?'first_name'([A-ZÀÈÌÒÙÁÉÍÓÚÝÂÊÎÔÛÃÑÕÄËÏÖÜŸÇÅa-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'][a-zàèìòùáéíóúýâêîôûãñõäëïöü¡¿çãñõ'\-]+))$@";
          $result = preg_match($regex, $name, $matches);
          if (!preg_match($regex, $name, $matches)) {
            // No match found.
            return FALSE;
          }
        }
      }
    }
  }
  // Set first character uppercase.
  $parts = array(
    'first_name',
    'last_name',
  );
  foreach ($parts as $part) {
    if (isset($matches[$part])) {
      $matches[$part] = trim($matches[$part]);
      if (!empty($matches[$part])) {
        $matches[$part] = ucfirst($matches[$part]);
      }
    }
  }
  return $matches;
}
