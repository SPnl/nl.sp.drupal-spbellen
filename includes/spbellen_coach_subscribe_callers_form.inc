<?php

function spbellen_coach_subscribe_callers_form($form, &$form_state) {

  $storage = !empty($form_state['storage']) ? $form_state['storage'] : array();

  // Preset default values.
  $fields = array(
    'first_name' => 'first_name',
    'middle_name' => 'middle_name',
    'last_name' => 'last_name',
    'phone' => 'phone',
    'postal_code' => 'postal_code',
  );
  foreach ($fields as $attribute => $form_key) {
    $default[$form_key] = '';
  }
  if (!empty($storage['caller'])) {
    $caller = $storage['caller'];
    foreach ($fields as $attribute => $form_key) {
      $values[$form_key] = (empty($caller->$attribute)) ? '' : $caller->$attribute;
      unset($form_state['input'][$form_key]);
      $form_state['values'][$form_key] = (empty($caller->$attribute)) ? '' : $caller->$attribute;
      $default[$form_key] = (empty($caller->$attribute)) ? '' : $caller->$attribute;
    }
  }

  $values = !empty($form_state['values']) ? $form_state['values'] : array();

  // Define form array.
  $form = array();
  $form['#prefix'] = '<div id="spbellen_ajax_form">';
  $form['#suffix'] = '</div>';

  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => 'E-mail',
    '#name' => 'email',
    '#required' => TRUE,
    '#ajax' => array(
      'wrapper' => 'spbellen_ajax_form',
      'callback' => 'spbellen_form_ajax_callback',
      'event' => 'change',
    ),
  );

  $form['first_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Voornaam',
    '#required' => TRUE,
    '#default_value' => empty($values['first_name']) ? $default['first_name'] : $values['first_name'],
  );

  $form['middle_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Tussenvoegsel',
    '#default_value' => empty($values['middle_name']) ? $default['middle_name'] : $values['middle_name'],
  );

  $form['last_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Achternaam',
    '#required' => TRUE,
    '#default_value' => empty($values['last_name']) ? $default['last_name'] : $values['last_name'],
  );

  $form['phone'] = array(
    '#type' => 'textfield',
    '#title' => '06-nummer',
    '#required' => TRUE,
    '#default_value' => empty($values['phone']) ? $default['phone'] : $values['phone'],
  );

  $form['postal_code'] = array(
    '#type' => 'textfield',
    '#title' => 'Postcode',
    '#default_value' => empty($values['postal_code']) ? $default['postal_code'] : $values['postal_code'],
  );

  $form['account_pass'] = array(
    '#type' => 'password_confirm',
    '#title' => 'Kies een wachtwoord',
  );

  // Feedback element.
  if (!empty($form_state['feedback'])) {
    $form['feedback'] = array(
      '#markup' => $form_state['feedback'],
    );
  }

  $form['caller_state'] = array(
    '#type' => 'checkboxes',
    '#options' => array('no_campaign_key_needed' => 'Mag zonder campagnecode aanmelden voor belcampagnes'),
  );

  $form['submit_account_data'] = array(
    '#type' => 'submit',
    '#value' => 'Verder',
    '#name' => 'submit_account_data',
    '#ajax' => array(
      'wrapper' => 'spbellen_ajax_form',
      'callback' => 'spbellen_form_ajax_callback',
    ),
  );

  return $form;
}

function spbellen_coach_subscribe_callers_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $storage = &$form_state['storage'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'email':
      $account = user_load_by_name($values['email']);
      if ($account) {
        $query = new EntityFieldQuery();
        $result = $query
          ->entityCondition('entity_type', 'caller')
          ->propertyCondition('caller_id', $account->uid)
          ->execute();
        if (isset($result['caller'])) {
          $caller = entity_load_single('caller', array_pop($result['caller'])->caller_id);
          $storage['caller'] = $caller;
        }
        $form_state['rebuild'] = TRUE;
      }
      break;
    case 'submit_account_data':
      $fields_to_trim = array(
        'first_name',
        'middle_name',
        'last_name',
        'phone',
        'postal_code',
        'email',
        'account_pass',
      );
      if (!empty($campaign->campaign_code)) {
        $fields_to_trim[] = 'campaign_code';
      }
      foreach ($fields_to_trim as $field_to_trim) {
        $form_state['values'][$field_to_trim] = trim($values[$field_to_trim]);
      }

      // Validate name fields.
      $name_types = array(
        'first_name',
        'last_name',
      );
      foreach ($name_types as $name_type) {
        if (!preg_match('/^(?=.*[a-z]).*$/', $values[$name_type])) {
          form_set_error($name_type, 'Niet alleen hoofdletters! (caps lock ingeschakeld?)');
          $error = TRUE;
        }
      }

      // Validatie 06-nummer.
      if (!preg_match('/^(((\\+31|0|0031)6){1}[1-9]{1}[0-9]{7})$/i', $values['phone'])) {
        form_set_error('phone', 'Vul een geldig 06-nummer in!');
        $error = TRUE;
      }

      // Validatie .
      if (!empty($values['postal_code'])) {
        if (!preg_match('/^[1-9][0-9]{3} ?[a-zA-Z]{2}$/', $values['postal_code'])) {
          form_set_error('phone', 'Vul een geldige postcode in!');
          $error = TRUE;
        }
      }

      if (!empty($values['account_pass'])) {
        // Validate password.
        $regex_tests = array(
          '[0-9]+' => 'Gebruik tenminste één cijfer in het wachtwoord',
          '[a-z]+' => 'Gebruik tenminste één kleine letter in het wachtwoord',
          '[A-Z]+' => 'Gebruik tenminste één hoofdletter in het wachtwoord',
          '\W+' => 'Gebruik tenminste één teken (bijv. !?+-= e.d.) in het wachtwoord',
        );
        foreach ($regex_tests as $regex => $feedback) {
          if (!preg_match('/' . $regex . '/', $values['account_pass'])) {
            $pass_feedback_array[] = $feedback;
            $error = TRUE;
            $pass_error = TRUE;
          }
        }
        if (strlen($values['account_pass']) < 6) {
          $pass_feedback_array[] = 'Het wachtwoord moet minimaal 6 tekens lang zijn';
          $error = TRUE;
          $pass_error = TRUE;
        }
        if (!empty($pass_error)) {
          $pass_feedback = implode('<br/>', $pass_feedback_array);
          form_set_error('account_pass', $pass_feedback);
        }
      }
      return;
      break;
    }
  }
}

function spbellen_coach_subscribe_callers_form_submit($form, &$form_state) {
  $values = &$form_state['values'];
  $storage = &$form_state['storage'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'submit_account_data':
      require_once DRUPAL_ROOT . '/includes/password.inc';
      $account = user_load_by_name($values['email']);
      if ($account) {
        require_once DRUPAL_ROOT . '/' . variable_get('password_inc', 'includes/password.inc');
        // Update existing account.
        //$account->name = $values['email'];
        $account->mail = $values['email'];
        if (!empty($values['account_pass'])) {
          $account->pass = user_hash_password($values['account_pass']);
        }
        $account->status = 1;
        $account->access = REQUEST_TIME;
        $account->roles = array();
        $account = user_save($account);
        drupal_set_message('Het account van ' . check_plain($values['first_name']) . ' is bijgewerkt.');
      }
      else {
        // Create new account.
        $user = array(
          'name' => $values['email'],
          'mail' => $values['email'],
          'pass' => $values['account_pass'],
          'status' => 1,
          'access' => REQUEST_TIME,
          'roles' => array(),
        );
        $account = user_save(NULL, $user);
        if (!empty($values['account_pass'])) {
          $pass = user_hash_password($values['account_pass']);
        }
        else {
          $pass = user_password();
        }
        $account->pass = $pass;
        $account = user_save($account);
        drupal_set_message('Er is een nieuw account aangemaakt voor ' . check_plain($values['first_name']) . ' .');
      }
      if (!empty($account)) {
        // Save caller data.
        $fields = array('first_name', 'middle_name', 'last_name');
        $name_parts = array();
        $caller_state = empty($values['caller_state']['no_campaign_key_needed']) ? 'registered' : 'master';
        $chapter_and_region = spbellen_get_chapter_and_region($values['postal_code']);
        foreach ($fields as $field) {
          if (!empty($values[$field])) {
            $name_parts[$field] = $values[$field];
          }
        }
        $display_name = implode(' ', $name_parts);
        $query = new EntityFieldQuery();
        $result = $query
          ->entityCondition('entity_type', 'caller')
          ->propertyCondition('caller_id', $account->uid)
          ->execute();
        if (isset($result['caller'])) {
          // Update existing data.
          $caller = entity_load_single('caller', array_pop($result['caller'])->caller_id);
          $caller->access_type = 'local';
          $caller->state = $caller_state;
          $caller->first_name = $values['first_name'];
          $caller->middle_name = $values['middle_name'];
          $caller->last_name = $values['last_name'];
          $caller->display_name = $display_name;
          $caller->phone = $values['phone'];
          $caller->postal_code = $values['postal_code'];
          if (!empty($chapter_and_region['afdeling_id'])) {
            $caller->afdeling = $chapter_and_region['afdeling'];
            $caller->afdeling_id = $chapter_and_region['afdeling_id'];
          }
          else {
            $caller->afdeling = '';
            $caller->afdeling_id = NULL;
          }
          if (!empty($chapter_and_region['regio_id'])) {
            $caller->regio = $chapter_and_region['regio'];
            $caller->regio_id = $chapter_and_region['regio_id'];
          }
          else {
            $caller->regio = '';
            $caller->regio_id = NULL;
          }
        }
        else {
          // Create new data.
          $caller_fields = array(
            'caller_id' => $account->uid,
            'access_type' => 'local',
            'state' => $caller_state,
            'first_name' => $values['first_name'],
            'middle_name' => $values['middle_name'],
            'last_name' => $values['last_name'],
            'display_name' => $display_name,
            'phone' => $values['phone'],
            'postal_code' => $values['postal_code'],
          );
          if (!empty($chapter_and_region['afdeling_id'])) {
            $caller_fields['afdeling'] = $chapter_and_region['afdeling'];
            $caller_fields['afdeling_id'] = $chapter_and_region['afdeling_id'];
          }
          else {
            $caller_fields['afdeling'] = '';
            $caller_fields['afdeling_id'] = NULL;
          }
          if (!empty($chapter_and_region['regio_id'])) {
            $caller_fields['regio'] = $chapter_and_region['regio'];
            $caller_fields['regio_id'] = $chapter_and_region['regio_id'];
          }
          else {
            $caller_fields['regio'] = '';
            $caller_fields['regio_id'] = NULL;
          }
          $caller = entity_create('caller', $caller_fields);
        }
        entity_save('caller', $caller);
        $form_state['finish'] = TRUE;
        $form_state['values'] = array();
        $form_state['input'] = array();
        $form_state['storage'] = array();
        $form_state['rebuild'] = TRUE;
      }
    }
  }
}
