<?php

function spbellen_overview() {
  $variables = array();
  $campaigns = spbellen_get_campaigns();
  foreach ($campaigns as $campaign_id => $campaign) {
    $webform = entity_load_single('node', $campaign->webform_id);
    $query = "SELECT COUNT(*) total, SUM(bl.civi_contact_id IS NOT NULL) blacklisted, SUM(bl.civi_contact_id IS NULL AND (result = 'answered_call_back' OR result = 'not_answered')) call_back, SUM(bl.civi_contact_id IS NULL AND result = 'answered_completed') completed, SUM(bl.civi_contact_id IS NULL AND result = 'wrong_number') wrong_number, SUM(bl.civi_contact_id IS NULL AND attempts = 0) not_contacted FROM {spbellen_targets} ta LEFT JOIN {spbellen_action} ac ON ta.last_action_id = ac.id LEFT {JOIN spbellen_blacklist} bl ON ta.civi_contact_id = bl.civi_contact_id WHERE campaign_id = :campaign_id;";
    $result = db_query($query, array(':campaign_id' => $campaign_id));
    $record = $result->fetchAssoc();
    $variables['campaign_stats'][$campaign_id] = $record;
    foreach ($record as $key => $value) {
      $variables['campaign_stats'][$campaign_id][$key.'_percentage'] = floor(100 * $value / $record['total']);
    }
    $variables['campaign_stats'][$campaign_id]['title'] = $webform->title;
  }
  return theme('spbellen_overview_theme', $variables);
}
