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
  $items['spbellen_auto_import'] = array(
    'callback' => 'spbellen_auto_import',
    'description' => 'Drush command to automaticly import a batch of contacts.',
    'aliases' => array('spbai'),
    'examples' => array(
      'Import a batch of contacts' => 'drush spbellen_auto_import',
      'Import a batch of contacts' => 'drush spbai',
    ),
  );
  return $items;
}

/**
 * Remove fake phone numbers.
 */
function spbellen_remove_fake_phone_numbers() {
  // Remove untargeted contact data.
  drush_print('Removing untargeted contact data.');
  $query = "DELETE cd FROM spbellen_contact_data cd LEFT JOIN spbellen_targets ta ON ta.contact_id = cd.contact_id WHERE ta.contact_id IS NULL";
  db_query($query);
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
        $pattern2 .= $j . $i;
      }
      $query = "DELETE cd, ta, ac FROM {spbellen_contact_data} cd LEFT JOIN {spbellen_targets} ta ON cd.contact_id = ta.contact_id LEFT JOIN {spbellen_action} ac ON ac.target_id = ta.id WHERE phone LIKE :pattern";
      db_query($query, array('pattern' => '%' . db_like($pattern1) . '%'));
      $query = "DELETE cd, ta, ac FROM {spbellen_contact_data} cd LEFT JOIN {spbellen_targets} ta ON cd.contact_id = ta.contact_id LEFT JOIN {spbellen_action} ac ON ac.target_id = ta.id WHERE phone LIKE :pattern";
      db_query($query, array('pattern' => '%' . db_like($pattern2) . '%'));
    }
  }
}

/**
 * Callback for hook_drush_command.
 */
function spbellen_merge_duplicates() {
  // Get duplicate phone numbers.
  drush_print('Merging duplicate contacts.');
  $query = "SELECT da1.phone phone FROM {spbellen_contact_data} da1 INNER JOIN {spbellen_contact_data} da2 ON da1.contact_id != da2.contact_id AND da1.phone = da2.phone WHERE da2.crm_contact_id IS NULL OR da1.crm_contact_id = da2.crm_contact_id LIMIT 3;";
  $result = db_query($query);
  if ($result) {
    while ($row = $result->fetchAssoc()) {
      if (!empty($row['phone'])) {
        drush_print('Merging contact');
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
        $same_contacts[$row['contact_id']] = $row['contact_id'];
      }
      if (!empty($same_contacts)) {
        drush_print('Same contacts:');
        $output = drush_format($same_contacts);
        drush_print($output);
        // Merge same contacts.
        spbellen_merge_contacts($same_contacts);
      }
    }
  }
}

/**
 * Import a batch of contacts.
 */
function spbellen_auto_import() {
  $importer_info = spbellen_importer_info();
  foreach ($importer_info as $importer_data) {
    if (!empty($importer_data['import_function'])) {
      $importer_data['import_function']();
    }
  }
}

/**
 * Merge contacts.
 */
function spbellen_merge_contacts($contact_ids) {
  // Find best contact.
  $best_contact_id = spbellen_find_best_contact($contact_ids);
  $best_contact_data = spbellen_get_contact_data($best_contact_id);
  // Merge contact data.
  foreach ($contact_ids as $contact_id) {
    if ($contact_id != $best_contact_id) {
      $contact_data = spbellen_get_contact_data($contact_id);
      drush_print("");
      drush_print('Phone: ' . $best_contact_data['phone']);
      drush_print('Merging contact ' . $contact_id . ' with ' . $best_contact_id);
      $output = drush_format($contact_data);
      drush_print($output);
      $output = drush_format($best_contact_data);
      drush_print($output);

      // Check if contacts share campaign.
      $query = "SELECT t1.campaign_id campaign_id, t1.contact_id contact_id, t1.id contact_target_id, ac1.result contact_result, t2.contact_id best_contact_id, t2.id best_contact_target_id, ac2.result best_contact_result FROM spbellen_targets t1 INNER JOIN spbellen_targets t2 ON t1.contact_id = :contact_id AND t2.contact_id = :best_contact_id AND t1.campaign_id = t2.campaign_id LEFT JOIN spbellen_action ac1 ON t1.last_action_id = ac1.id LEFT JOIN spbellen_action ac2 ON t2.last_action_id = ac2.id";
      $result = db_query($query, array('contact_id' => $contact_id, 'best_contact_id' => $best_contact_id));
      if ($result) {
        if ($result->rowCount()) {
          // Contacts share campaign, remove one of the targets and actions that belong to it.
          while ($row = $result->fetchAssoc()) {
            if (empty($row['campaign_id'])) {
              drush_print('Empty!');
            }
            $output = drush_format($row);
            drush_print($output);
            $campaign_id = $row['campaign_id'];
            $contact_target_id = $row['contact_target_id'];
            $contact_result = $row['contact_result'];
            $best_contact_target_id = $row['best_contact_target_id'];
            $best_contact_result = $row['best_contact_result'];
            // Find best target.
            if (empty($contact_result)) {
              $keep_target_id = $best_contact_target_id;
              $remove_target_id = $contact_target_id;
            }
            elseif (empty($best_contact_result)) {
              $keep_target_id = $contact_target_id;
              $remove_target_id = $best_contact_target_id;
            }
            elseif ($best_contact_result === 'answered_completed') {
              $keep_target_id = $best_contact_target_id;
              $remove_target_id = $contact_target_id;
            }
            elseif ($contact_result === 'answered_completed') {
              $keep_target_id = $contact_target_id;
              $remove_target_id = $best_contact_target_id;
            }
            elseif ($best_contact_result === 'call_appointment') {
              $keep_target_id = $best_contact_target_id;
              $remove_target_id = $contact_target_id;
            }
            elseif ($contact_result === 'call_appointment') {
              $keep_target_id = $contact_target_id;
              $remove_target_id = $best_contact_target_id;
            }
            elseif ($best_contact_result === 'answered_call_back') {
              $keep_target_id = $best_contact_target_id;
              $remove_target_id = $contact_target_id;
            }
            elseif ($contact_result === 'answered_call_back') {
              $keep_target_id = $contact_target_id;
              $remove_target_id = $best_contact_target_id;
            }
            elseif ($best_contact_result === 'answered_blacklist') {
              $keep_target_id = $best_contact_target_id;
              $remove_target_id = $contact_target_id;
            }
            elseif ($contact_result === 'answered_blacklist') {
              $keep_target_id = $contact_target_id;
              $remove_target_id = $best_contact_target_id;
            }
            else {
              $keep_target_id = $best_contact_target_id;
              $remove_target_id = $contact_target_id;
            }
            // Remove target with less interesting result.
            $query = "DELETE ta, ac FROM spbellen_targets ta LEFT JOIN spbellen_action ac ON ac.target_id = ta.id WHERE ta.id = $remove_target_id";
            drush_print($query);
            $query = "DELETE ta, ac FROM spbellen_targets ta LEFT JOIN spbellen_action ac ON ac.target_id = ta.id WHERE ta.id = :remove_target_id";
            db_query($query, array('remove_target_id' => $remove_target_id));
            // Assign best target to best contact.
            $query = "UPDATE spbellen_targets ta SET ta.contact_id = $best_contact_id WHERE ta.id = $keep_target_id";
            drush_print($query);
            $query = "UPDATE spbellen_targets ta SET ta.contact_id = :best_contact_id WHERE ta.id = :keep_target_id";
            db_query($query, array('best_contact_id' => $best_contact_id, 'keep_target_id' => $keep_target_id));
            // Delete inferior contact data.
            $query = "DELETE FROM spbellen_contact_data WHERE contact_id = $contact_id";
            drush_print($query);
            $query = "DELETE FROM spbellen_contact_data WHERE contact_id = :contact_id";
            db_query($query, array('contact_id' => $contact_id));
          }
        }
        else {
          // Contacts do not share campaign, just change contact_id.
          $query = "UPDATE spbellen_targets SET contact_id = $best_contact_id WHERE contact_id = $contact_id";
          drush_print($query);
          $query = "UPDATE spbellen_targets SET contact_id = :best_contact_id WHERE contact_id = :contact_id";
          db_query($query, array('best_contact_id' => $best_contact_id, 'contact_id' => $contact_id));
          $query = "DELETE FROM spbellen_contact_data WHERE contact_id = $contact_id";
          drush_print($query);
          $query = "DELETE FROM spbellen_contact_data WHERE contact_id = :contact_id";
          db_query($query, array('contact_id' => $contact_id));
        }
      }
      drush_print('-------------------------------------');
    }
  }
}

/**
 * Find best contact.
 */
function spbellen_find_best_contact($contact_ids) {
  foreach ($contact_ids as $contact_id) {
    $contact_data = spbellen_get_contact_data($contact_id);
    $scores[$contact_id] = 0;
    if (!empty($contact_data['crm_contact_id'])) {
      $scores[$contact_id] = 3;
    }
    foreach ($contact_data as $value) {
      if (!empty($value)) {
        $scores[$contact_id]++;
      }
    }
  }
  arsort($scores);
  return key($scores);
}
