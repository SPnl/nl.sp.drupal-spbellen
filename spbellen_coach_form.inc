<?php

function spbellen_coach_add_select_campaign_form_validate(&$form_state) {
  $values = $form_state['values'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'select_campaign':
      $_SESSION['spbellen']['coach_selected_campaign'] = $values['select_campaign'];
      break;
    }
  }
}

function spbellen_coach_get_caller($campaign_id, $state) {
  $query = "SELECT us.uid uid FROM spbellen_caller cal INNER JOIN spbellen_local_access la ON cal.caller_id = la.uid INNER JOIN users us ON la.uid = us.uid INNER JOIN spbellen_campaign cam ON la.campaign_id = cam.id WHERE la.campaign_id = :campaign_id AND cal.state = :state AND caller_check = 1 LIMIT 1";
  $result = db_query($query, array(':campaign_id' => $campaign_id, ':state' => $state));
  if (!empty($result)) {
    $caller = entity_load_single('caller', $result->fetchField(0));
    if (!empty($caller))
      return $caller;
  }
  return FALSE;
}
