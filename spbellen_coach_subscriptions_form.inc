<?php

function spbellen_coach_subscriptions_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'spbellen', 'spbellen_coach_form');
  $values = !empty($form_state['values']) ? $form_state['values'] : array();
  $storage = !empty($form_state['storage']) ? $form_state['storage'] : array();
  $campaign_id = empty($_SESSION['spbellen']['coach_selected_campaign']) ? FALSE : $_SESSION['spbellen']['coach_selected_campaign'];

  $form = array();
  $form['#prefix'] = '<div id="spbellen_ajax_form">';
  $form['#suffix'] = '</div>';
  if ($campaign_id === FALSE) {
    spbellen_coach_add_select_campaign_form($form);
  }
  else {
    $caller = spbellen_coach_get_caller($campaign_id, 'subscribed');
    if (!empty($caller)) {
      $form['caller'] = array(
        '#markup' => '<h1>' . check_plain($caller->phone) . '</h1><h2>'. check_plain($caller->display_name).'</h2>',
        '#attributes' => array('class' => array('test')),
      );
    }
    else {
      $form['no_caller'] = array(
        '#markup' => '<p>Er is geen beller gevonden.</p>',
      );
    }
  }

  return $form;
}

function spbellen_coach_subscriptions_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $storage = &$form_state['storage'];

  spbellen_coach_add_select_campaign_form_validate($form_state);

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'xxxx':
      break;
    }
  }
}
