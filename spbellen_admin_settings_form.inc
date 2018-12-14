<?php

function spbellen_settings_form($form, &$form_state) {

  $form['spbellen_general_settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'Algemene instellingen',
  );
  $options = array(
    '1' => '1',
    '2' => '2',
    '3' => '3',
    '4' => '4',
    '5' => '5',
    '6' => '6',
    '7' => '7',
    '8' => '8',
    '9' => '9',
    '10' => '10',
  );
  $form['spbellen_general_settings']['spbellen_max_attempts'] = array(
    '#title' => 'Maximaal aantal belpogingen',
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => variable_get('spbellen_max_attempts', '5'),
  );

  // Click2dial Voclarion settings.
  $form['spbellen_click2dial'] = array(
    '#type' => 'fieldset',
    '#title' => 'Click to dial instellingen',
  );
  $form['spbellen_click2dial']['general'] = array(
    '#type' => 'fieldset',
    '#title' => 'Algemeen',
  );
  $form['spbellen_click2dial']['general']['spbellen_click2dial_shortname'] = array(
    '#type' => 'textfield',
    '#title' => 'Shortname',
    '#description' => 'Verkorte bedrijfsnaam die in de Voclarion gebruikt wordt.',
    '#default_value' => variable_get('spbellen_click2dial_shortname', ''),
  );
  $form['spbellen_click2dial']['general']['spbellen_click2dial_location_ip'] = array(
    '#type' => 'textfield',
    '#title' => 'IP adres van locatie waar gebeld wordt',
    '#default_value' => variable_get('spbellen_click2dial_location_ip', ''),
  );
  $form['spbellen_click2dial']['general']['spbellen_click2dial_asterix_domain'] = array(
    '#type' => 'textfield',
    '#title' => 'Domein van telefooncentrale',
    '#default_value' => variable_get('spbellen_click2dial_asterix_domain', ''),
  );
  $form['spbellen_click2dial']['caller_ids'] = array(
    '#type' => 'fieldset',
    '#title' => 'Belnummers',
  );
  $quantity = variable_get('click2dial_call_numbers_quantity', 2);
  for ($nr = 1; $nr <= $quantity; $nr ++) {
    $form['spbellen_click2dial']['caller_ids']['caller_id_' . $nr] = array(
      '#type' => 'fieldset',
      '#title' => 'Nummer ' . $nr,
    );
    $form['spbellen_click2dial']['caller_ids']['caller_id_' . $nr]['spbellen_click2dial_callerIDName_' . $nr] = array(
      '#type' => 'textfield',
      '#title' => 'Naam',
      '#default_value' => variable_get('spbellen_click2dial_callerIDName_' . $nr, ''),
    );
    $form['spbellen_click2dial']['caller_ids']['caller_id_' . $nr]['spbellen_click2dial_forceclip_' . $nr] = array(
      '#type' => 'textfield',
      '#title' => 'Nummer',
      '#default_value' => variable_get('spbellen_click2dial_forceclip_' . $nr, ''),
    );
  }
  $form['spbellen_text_settings'] = array(
    '#type' => 'fieldset',
    '#title' => 'Tekst instellingen',
  );
  $default = variable_get('spbellen_general_start_page', array('value' => '<p>Welkom bij de SP Beltool.</p><p>Als je zo gaat bellen krijg je toegang tot persoonlijke gegevens van mensen. Ga hier zorgvuldig mee om. Het is niet toegestaan persoonsgegevens te verzamelen door deze op te op te schrijven of anderszins op te slaan. Een uitzondering hierop is als er bij een gesprek iets misgaat, of er om een andere reden iets moet worden doorgegeven over een contact aan de belcoördinator.</p>'));
  $form['spbellen_text_settings']['spbellen_general_start_page'] = array(
    '#type' => 'text_format',
    '#title' => 'Entree pagina',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
  );
  $default = variable_get('spbellen_campaign_select_intro', array('value' => '<p>Kies hieronder een campagne waarvoor je wil bellen.</p>'));
  $form['spbellen_text_settings']['spbellen_campaign_select_intro'] = array(
    '#type' => 'text_format',
    '#title' => 'Introductie loginpagina',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
  );
  $default = variable_get('spbellen_login_intro_campaign_code', array('value' => '<p>Om in te loggen voor deze belcampagne is je e-mailadres en een campagne code vereist. Heb je de campagnecode niet, neem dan contact op met de belcoördinator.</p>'));
  $form['spbellen_text_settings']['spbellen_login_intro_campaign_code'] = array(
    '#type' => 'text_format',
    '#title' => 'Introductie eerste stap inloggen met campagnecode',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
  );
  $default = variable_get('spbellen_login_intro_no_campaign_code', array('value' => '<p>Om in te loggen voor deze belcampagne is je e-mailadres nodig.</p>'));
  $form['spbellen_text_settings']['spbellen_login_intro_no_campaign_code'] = array(
    '#type' => 'text_format',
    '#title' => 'Introductie eerste stap inloggen zonder campagnecode',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
  );
  $default = variable_get('spbellen_login_intro_first_visit', array('value' => '<p>Je krijgt bij het gebruik van de beltool toegang tot privacy gevoelige informatie, daarom stellen we hieronder een aantal vragen zodat we kunnen controleren wie je bent. Daarnaast moet je een wachtwoord in te stellen voor de beltool. Vergeet dit wachtwoord niet, je hebt het de volgende keer dat je gaat bellen voor een belcampagne weer nodig om in te loggen.</p>'));
  $form['spbellen_text_settings']['spbellen_login_intro_first_visit'] = array(
    '#type' => 'text_format',
    '#title' => 'Introductie inloggen eerste bezoek',
    '#format' => isset($default['format']) ? $default['format'] : NULL,
    '#default_value' => isset($default['value']) ? $default['value'] : '',
  );
  $default = variable_get('spbellen_login_intro_password', array('value' => '<p>Je hebt de eerste keer dat u de SP Beltool gebruikte een wachtwoord ingesteld. Vul dat wachtwoord hieronder in.</p>'));
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
