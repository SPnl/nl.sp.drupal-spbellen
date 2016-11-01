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
  // Click2dial Voclarion settings.
  $form['spbellen_click2dial'] = array(
    '#type' => 'fieldset',
    '#title' => 'Click to dial instellingen',
  );
  $form['spbellen_click2dial']['spbellen_click2dial_shortname'] = array(
    '#type' => 'textfield',
    '#title' => 'Shortname',
    '#description' => 'Verkorte bedrijfsnaam die in de Voclarion gebruikt wordt.',
    '#default_value' => variable_get('spbellen_click2dial_shortname', ''),
    '#required' => TRUE,
  );
  $form['spbellen_click2dial']['spbellen_click2dial_forceclip'] = array(
    '#type' => 'textfield',
    '#title' => 'CallerID',
    '#description' => 'Dit is het telefoonnummer dat gebelde ziet.',
    '#default_value' => variable_get('spbellen_click2dial_forceclip', ''),
  );
  $form['spbellen_click2dial']['spbellen_click2dial_CallerIDName'] = array(
    '#type' => 'textfield',
    '#title' => 'CallerIDName',
    '#description' => 'De naam die gebelde ziet.',
    '#default_value' => variable_get('spbellen_click2dial_CallerIDName', ''),
  );
  $form['spbellen_click2dial']['spbellen_click2dial_location_ip'] = array(
    '#type' => 'textfield',
    '#title' => 'IP adres van locatie waar gebeld wordt',
    '#default_value' => variable_get('spbellen_click2dial_location_ip', ''),
    '#required' => TRUE,
  );
  $form['spbellen_click2dial']['spbellen_click2dial_asterix_ip'] = array(
    '#type' => 'textfield',
    '#title' => 'IP adres van telefooncentrale',
    '#default_value' => variable_get('spbellen_click2dial_asterix_ip', ''),
    '#required' => TRUE,
  );
  $form['spbellen_text_settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'Tekst instellingen',
  );
  $default = variable_get('spbellen_general_start_page', '');
  $form['spbellen_text_settings']['spbellen_general_start_page'] = array(
    '#type' => 'text_format',
    '#title' => 'Entree pagina',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
  );
  $default = variable_get('spbellen_campaign_select_intro', '');
  $form['spbellen_text_settings']['spbellen_campaign_select_intro'] = array(
    '#type' => 'text_format',
    '#title' => 'Introductie loginpagina',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
  );
  $default = variable_get('spbellen_login_intro_campaign_code', '');
  $form['spbellen_text_settings']['spbellen_login_intro_campaign_code'] = array(
    '#type' => 'text_format',
    '#title' => 'Introductie eerste stap inloggen met campagnecode',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
  );
  $default = variable_get('spbellen_login_intro_no_campaign_code', '');
  $form['spbellen_text_settings']['spbellen_login_intro_no_campaign_code'] = array(
    '#type' => 'text_format',
    '#title' => 'Introductie eerste stap inloggen zonder campagnecode',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
  );
  $default = variable_get('spbellen_login_intro_first_visit', '');
  $form['spbellen_text_settings']['spbellen_login_intro_first_visit'] = array(
    '#type' => 'text_format',
    '#title' => 'Introductie inloggen eerste bezoek',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
  );
  $default = variable_get('spbellen_login_intro_password', '');
  $form['spbellen_text_settings']['spbellen_login_intro_password'] = array(
    '#type' => 'text_format',
    '#title' => 'Introductie inloggen wachtwoord',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
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
