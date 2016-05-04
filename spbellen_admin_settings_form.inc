<?php

function spbellen_settings_form($form, &$form_state) {
  $campaigns_list = spbellen_get_campaigns_list();
  $webforms_list = spbellen_get_webforms_list();
  $default = variable_get('spbellen_intro_webform', '');
  $form['intro_webform'] = array(
    '#type' => 'select',
    '#title' => 'Kies het spbellen introformulier',
    '#options' => $webforms_list,
    '#empty_option' => 'Kies:',
    '#required' => TRUE,
    '#default_value' => $default,
  );
  $default = variable_get('spbellen_active_campaign', '');
  $form['campaign'] = array(
    '#title' => 'Selecteer de actieve campagne',
    '#type' => 'select',
    '#options' => $campaigns_list,
    '#empty_option' => 'Geen',
    '#default_value' => $default,
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Sla op',
  );
  return $form;
}

function spbellen_settings_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  if (!empty($values['campaign'])) {
    variable_set('spbellen_active_campaign', $values['campaign']);
    drupal_set_message('Actieve campagne is opgeslagen.');
  }
  else {
    drupal_set_message('Er is geen actieve campagne.', 'warning');
  }
  if (!empty($values['intro_webform'])) {
    variable_set('spbellen_intro_webform', $values['intro_webform']);
    drupal_set_message('Het introformulier is ingesteld.');
  }
  else {
    drupal_set_message('Er is geen introformulier ingesteld.', 'warning');
  }
}
