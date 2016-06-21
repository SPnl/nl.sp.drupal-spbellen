<?php

function spbellen_settings_form($form, &$form_state) {
  $form = array();
  $form['no_settings'] = array(
    '#markup' => '<p>Er zijn nog geen instellingen beschikbaar.</p>',
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
}
