<?php

function spbellen_settings_form($form, &$form_state) {
  // Set e-mail settings.
  $form['spbellen_email_settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'E-mail instellingen',
  );
  $form['spbellen_email_settings']['spbellen_corrections_email_address'] = array(
    '#type' => 'textfield',
    '#title' => 'Correcties contactgegevens sturen naar:',
    '#default_value' => variable_get('spbellen_corrections_email_address', ''),
  );
  return system_settings_form($form);
}

function spbellen_settings_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  if (!empty($values['spbellen_corrections_email_address'])) {
    if (! valid_email_address($values['spbellen_corrections_email_address'])) {
      form_set_error('spbellen_corrections_email_address', 'Vul een geldig e-mailadres in!');
    }
  }
}
