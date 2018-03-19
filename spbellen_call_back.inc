<?php

function spbellen_call_back_list() {
  $call_back_list_data = spbellen_get_call_back_list_data();
  $variables = array(
    'data' => spbellen_get_call_back_list_data(),
  );
  return theme('spbellen_call_back_list_theme', $variables);
}

function spbellen_get_call_back_list_data() {
  $data = array();
  $query = "SELECT id FROM {spbellen_campaign} WHERE disabled = 0";
  $campaigns = db_query($query);
  if ($campaigns) {
    while ($campaign = $campaigns->fetchAssoc()) {
      $query = "SELECT ta.id as target_id, ta.campaign_id as campaign_id, call_back_time, display_name, phone FROM {spbellen_targets} ta INNER JOIN {spbellen_contact_data} cd ON ta.contact_id = cd.contact_id LEFT JOIN {spbellen_action} ac ON ta.last_action_id = ac.id LEFT JOIN {spbellen_blacklist} bl ON ta.contact_id = bl.contact_id WHERE ta.state = 1 AND result = 'call_appointment' AND call_back_priority = 1 AND attempts < 10 AND bl.contact_id IS NULL AND campaign_id = :cid AND (occupied = 0 OR (occupied = 1 AND ta.date < :occupied_time)) ORDER BY call_back_time ASC, attempts ASC, date ASC";
      $call_back_data = db_query($query, array(
          ':cid' => $campaign['id'],
          ':occupied_time' => time() - 1800,
      ));
      if ($call_back_data) {
        while ($call_back = $call_back_data->fetchAssoc()) {
          foreach ($call_back as $key => $value) {
            $clean_call_back[$key] = check_plain($value);
          }
          $data[$campaign['id']]['call_back_data'][] = $clean_call_back;
        }
      }
    }
  }
  foreach ($data as $campaign_id => $call_back) {
    $campaign = entity_load_single('campaign', $campaign_id);
    $data[$campaign_id]['campaign']['title'] = check_plain($campaign->title);
  }
  return $data;
}

function spbellen_call_back_contact($target_id) {
  global $user;
  if (!empty($target_id)) {
    // Unclaim current target.
    db_update('spbellen_targets')
      ->fields(array(
        'occupied' => 0,
        'date' => time(),
        'caller_id' => NULL,
      ))
      ->condition('caller_id', $user->uid)
      ->condition('occupied', 1)
      ->execute();

    // Claim new target.
    db_update('spbellen_targets')
      ->fields(array(
        'occupied' => 1,
        'date' => time(),
        'caller_id' => $user->uid,
      ))
      ->condition('id', $target_id)
      ->execute();
  }

  // Get campaign.
  $target = entity_load_single('target', $target_id);
  $campaign_id = $target->campaign_id;
  $campaign = entity_load_single('campaign', $campaign_id);
  drupal_goto('node/' . $campaign->webform_id);
}
