<?php

function spbellen_subscription_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'content', 'includes/password');

  // Store campaign in form.
  if (!isset($form_state['campaign'])) {
    // Get campaign from path.
    $path = current_path();
    $campaign_alias = strrchr($path, '/');
    $webform_alias = 'bellen/campagne' . $campaign_alias;
    $webform_path = drupal_get_normal_path($webform_alias);
    $webform_id = arg(1, $webform_path);
    $webform = node_load($webform_id);
    $campaign = spbellen_get_campaign($webform_id);
    $form_state['webform_alias'] = $webform_alias;
    $form_state['campaign'] = $campaign;
    $form_state['webform'] = $webform;
  }

  // Attach css.
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'spbellen') . '/css/spbellen_subscription_form.css',
  );

  // Feedback element.
  if (!empty($form_state['feedback'])) {
    $form['feedback'] = array(
      '#markup' => $form_state['feedback'],
    );
  }

  if (empty($form_state['success'])) {
    // Email field.
    $form['email'] = array(
      '#type' => 'textfield',
      '#title' => 'E-mail',
      '#default_value' => empty($form_state['values']['email']) ? '' : $form_state['values']['email'],
      '#required' => TRUE,
      '#disabled' => isset($form_state['has_account']),
    );

    // Campaign code field.
    if (!empty($form_state['campaign']->campaign_code)) {
      $form['campaign_code'] = array(
        '#type' => 'password',
        '#title' => 'Campagne code',
        '#default_value' => empty($form_state['values']['campaign_code']) ? '' : $form_state['values']['campaign_code'],
        '#required' => TRUE,
        '#disabled' => isset($form_state['has_account']),
        '#suffix' => !empty($form_state['campaign_code_feedback']) ? $form_state['campaign_code_feedback'] : '',
      );
    }

    // Submit for account check.
    if (!isset($form_state['has_account'])) {
      $form['submit_has_account'] = array(
        '#type' => 'button',
        '#value' => 'Verder',
        '#name' => 'submit_has_account',
      );
    }
    else {
      // Has no account.
      if ($form_state['has_account'] === 0) {

        $form['first_name'] = array(
          '#type' => 'textfield',
          '#title' => 'Voornaam',
          '#required' => TRUE,
        );

        $form['middle_name'] = array(
          '#type' => 'textfield',
          '#title' => 'Tussenvoegsel',
        );

        $form['last_name'] = array(
          '#type' => 'textfield',
          '#title' => 'Achternaam',
          '#required' => TRUE,
        );

        $form['phone'] = array(
          '#type' => 'textfield',
          '#title' => '06-nummer',
          '#required' => TRUE,
        );

        $form['account_pass'] = array(
          '#type' => 'password_confirm',
          '#title' => 'Kies een wachtwoord',
          '#required' => TRUE,
        );

        $form['submit_account_data'] = array(
          '#type' => 'submit',
          '#value' => 'Verder',
          '#name' => 'submit_account_data',
        );
      }
      else {
        // Check pass.
        $form['account_pass'] = array(
          '#type' => 'password',
          '#title' => 'Wachtwoord',
        );
        $form['submit_account_pass'] = array(
          '#type' => 'button',
          '#value' => 'Verder',
          '#name' => 'submit_account_pass',
        );
      }
    }
  }

  return $form;
}

function spbellen_subscription_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $campaign = $form_state['campaign'];
  $webform = $form_state['webform'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'submit_has_account':
      // Trim fields.
      $form_state['values']['email'] = trim($values['email']);
      if (!empty($campaign->campaign_code)) {
        $form_state['values']['campaign_code'] = trim($values['campaign_code']);
      }

      // Validatie e-mail adres.
      if (! valid_email_address($values['email'])) {
        form_set_error('email', 'Vul een geldig e-mailadres in!');
        $error = TRUE;
      }

      // Check campaign code.
      if (!empty($campaign->campaign_code)) {
        // Check campaign code.
        if ($values['campaign_code'] !== $campaign->campaign_code) {
          unset($form_state['input']['campaign_code']);
          form_set_error('campaign_code', 'De campagnecode is niet correct!');
          $error = TRUE;
        }
      }
      if (!empty($error)) {
        return;
      }
      $account = user_load_by_name($values['email']);
      if (!empty($account)) {
        $form_state['account'] = $account;
        $form_state['has_account'] = 1;
      }
      else {
        $form_state['has_account'] = 0;
      }
      break;
    case 'submit_account_pass':
      $form_state['values']['account_pass'] = trim($values['account_pass']);
      $account = $form_state['account'];
      if (!empty($account)) {
        if (user_check_password($values['account_pass'], $account)) {
          global $user;
          $user = user_load($account->uid);
          user_login_finalize($account);
          $query = new EntityFieldQuery;
          $result = $query
            ->entityCondition('entity_type', 'local_access')
            ->propertyCondition('uid', $account->uid)
            ->propertyCondition('campaign_id', $campaign->id)
            ->execute();
          if (empty($result)) {
            $local_access = entity_create('local_access', array('uid' => $account->uid, 'campaign_id' => $campaign->id));
            entity_save('local_access', $local_access);
          }
          drupal_goto($form_state['webform_alias']);
        }
        else {
          form_set_error('account_pass', 'Het wachtwoord is niet correct');
          return;
        }
      }
      break;
    case 'submit_account_data':
      $fields_to_trim = array(
        'first_name',
        'middle_name',
        'last_name',
        'phone',
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
      if (strlen($values['account_pass'] < 6)) {
        $pass_feedback_array[] = 'Het wachtwoord moet minimaal 6 tekens lang zijn';
        $error = TRUE;
        $pass_error = TRUE;
      }
      if (!empty($pass_error)) {
        $pass_feedback = implode('<br/>', $pass_feedback_array);
        form_set_error('account_pass', $pass_feedback);
      }
      if (!empty($error)) {
        return;
      }
      else {
        // Create new account.
        $user = array(
          'name' => $values['email'],
          'mail' => $values['email'],
          'pass' => $values['account_pass'],
          'status' => 0,
          'field_first_name' => array(LANGUAGE_NONE => array(array('value' => $values['first_name']))),
          'field_middle_name' => array(LANGUAGE_NONE => array(array('value' => $values['middle_name']))),
          'field_last_name' => array(LANGUAGE_NONE => array(array('value' => $values['last_name']))),
          'field_phone_primary' => array(LANGUAGE_NONE => array(array('value' => $values['phone']))),
          'field_email_primary' => array(LANGUAGE_NONE => array(array('value' => $values['email']))),
          'access' => REQUEST_TIME,
          'roles' => array(),
        );
        $account = user_save('', $user);
        if (!empty($account)) {
          global $base_url;
          $hash = md5(uniqid(rand(), true));
          $url = $base_url . '/aanmelden/bevestig/' . $hash;
          $site_email = variable_get('site_mail', '');
          $params = array(
            'name' => $values['first_name'],
            'url' => $url,
            'webform_title' => $webform->title,
          );
          $result = drupal_mail('spbellen', 'register_confirmation_required', $account->mail, user_preferred_language($account), $params, $site_email);
          $local_access = entity_create('local_access', array(
            'uid' => $account->uid,
            'campaign_id' => $campaign->id,
            'hash' => $hash,
          ));
          entity_save('local_access', $local_access);
          $form_state['success'] = TRUE;
          $form_state['feedback'] = '<p>Er is een account aangemaakt, en een e-mail verzonden met instructies om je account te activeren.</p>';
        }
      }
      break;
    }
  }
  $form_state['rebuild'] = TRUE;
}

function spbellen_confirm_account($hash) {
  global $base_url;
  $query = new EntityFieldQuery();
  $result = $query
    ->entityCondition('entity_type', 'local_access')
    ->propertyCondition('hash', $hash)
    ->execute();
  if (isset($result['local_access'])) {
    $local_access_item_id = array_pop($result['local_access'])->id;
    $local_access_item = entity_load_single('local_access', $local_access_item_id);
    $local_access_item->hash = '';
    entity_save('local_access', $local_access_item);
    $campaign = entity_load_single('campaign', $local_access_item->campaign_id);
    global $user;
    $user = user_load($local_access_item->uid);
    if (empty($user->status)) {
      $user->status = 1;
      user_save($user);

      // Send confirmation mail.
      $webform = node_load('webform', $campaign->webform_id);
      $alias = spbellen_get_campaign_form_alias($campaign->webform_id);
      $site_email = variable_get('site_mail', '');
      $params = array(
        'name' => $user->field_first_name['und'][0]['value'],
        'url' => $base_url . '/aanmelden/campagne/' . $alias,
        'webform_title' => $webform->title,
      );
      $result = drupal_mail('spbellen', 'register_confirmation_success', $user->mail, user_preferred_language($user), $params, $site_email);
      drupal_set_message('De aanmelding is bevestigd');

      // Login user.
      user_login_finalize($account);
      drupal_goto('node/' . $campaign->webform_id);
    }
  }
  drupal_set_message('Het was niet mogelijk de aanmelding te bevestigen', 'error');
  drupal_goto('/bellen/error');
}
