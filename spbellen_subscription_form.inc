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
    $form_state['campaign_alias'] = $campaign_alias;
    $form_state['webform_alias'] = $webform_alias;
    $form_state['campaign'] = $campaign;
    $form_state['webform'] = $webform;
  }

  // Attach css.
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'spbellen') . '/css/spbellen_subscription_form.css',
  );

  // User already logged in?
  global $user;
  if ($user->uid !== 0) {
    if (spbellen_campaign_form_access($webform_id)) {
      drupal_goto($webform_alias);
    }
    else {
      $form['feedback'] = array(
        '#markup' => '<p>Je bent reeds aangemeld voor een andere campagne, klik hieronder om uit te loggen zodat je je kan aanmelden voor deze campagne.</p>',
      );

      $form['logout'] = array(
        '#type' => 'button',
        '#name' => 'logout',
        '#value' => 'Opnieuw aanmelden',
      );
    }
  }
  else {

    // Feedback element.
    if (!empty($form_state['feedback'])) {
      $form['feedback'] = array(
        '#markup' => $form_state['feedback'],
      );
    }

    if (empty($form_state['finish'])) {
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
          '#type' => 'textfield',
          '#title' => 'Campagne code',
          '#default_value' => empty($form_state['values']['campaign_code']) ? '' : $form_state['values']['campaign_code'],
          '#required' => TRUE,
          '#disabled' => isset($form_state['has_account']),
          '#suffix' => !empty($form_state['campaign_code_feedback']) ? $form_state['campaign_code_feedback'] : '',
          '#attributes' => array(
            'autocomplete' => 'off',
          ),
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

          $form['first_time'] = array(
            '#markup' => '<p>Welkom!</p><p>Je meld je voor het eerst aan voor een belcampagne, daarom vragen we nu een aantal gegevens.</p>',
          );

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

          $form['back_account_data'] = array(
            '#type' => 'button',
            '#value' => 'Terug',
            '#name' => 'back_account_data',
          );

        }
        else {
          // Check pass.
          $form['account_pass'] = array(
            '#type' => 'password',
            '#title' => 'Wachtwoord',
          );

          $form['submit_account_pass'] = array(
            '#type' => 'submit',
            '#value' => 'Verder',
            '#name' => 'submit_account_pass',
          );

          if (isset($form_state['account_pass_error'])) {
            $form['submit_pass_reset'] = array(
              '#type' => 'submit',
              '#value' => 'Wachtwoord aanpassen',
              '#name' => 'submit_pass_reset',
            );
          }

          $form['back_account_pass'] = array(
            '#type' => 'button',
            '#value' => 'Terug',
            '#name' => 'back_account_pass',
          );
        }
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
    case 'logout':
      $alias = $form_state['campaign_alias'];
      watchdog('user', 'Session closed for %name.', array('%name' => $user->name));
      module_invoke_all('user_logout', $user);
      // Destroy the current session, and reset $user to the anonymous user.
      session_destroy();
      drupal_goto('aanmelden/campagne' . $alias);
      break;
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

      // Store variables for flood control.
      $form_state['values']['name'] = $values['email'];
      // Register flood event.
      // Custom stripped pass check function based on user_login_authenticate_validate.
      spbellen_custom_login_authenticate_validate($form, $form_state);
      // Custom stripped function based on user_login_final_validate, no pass check.
      spbellen_custom_login_final_validate($form, $form_state);
      break;
    case 'back_account_pass':
      drupal_goto(request_path());
      break;
    case 'submit_pass_reset':
      drupal_goto('user/password');
      break;
    case 'submit_account_pass':
      // Store variables for flood control.
      $form_state['values']['name'] = $values['email'];
      $form_state['values']['pass'] = $values['account_pass'];

      // Flood control and check if user password is ok.
      user_login_authenticate_validate($form, $form_state);
      user_login_final_validate($form, $form_state);
      return;
      break;
    case 'back_account_data':
      unset($_SESSION['messages']['error']);
      drupal_goto(request_path());
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
        form_set_error('phone', 'Vul een geldig 06-nummer in (bijv. 0612345678)!');
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
      if (strlen($values['account_pass']) < 6) {
        $pass_feedback_array[] = 'Het wachtwoord moet minimaal 6 tekens lang zijn';
        $error = TRUE;
        $pass_error = TRUE;
      }
      if (!empty($pass_error)) {
        $pass_feedback = implode('<br/>', $pass_feedback_array);
        form_set_error('account_pass', $pass_feedback);
      }
      return;
      break;
    }
  }
  $form_state['rebuild'] = TRUE;
}

function spbellen_subscription_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $campaign = $form_state['campaign'];
  $webform = $form_state['webform'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'submit_account_pass':
      $form_state['values']['account_pass'] = trim($values['account_pass']);
      $account = $form_state['account'];
      if (!empty($account)) {
        if (user_check_password($values['account_pass'], $account)) {
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
          global $user;
          $user = user_load($account->uid);
          user_login_finalize($account);
          drupal_goto($form_state['webform_alias']);
        }
        else {
          $form_state['account_pass_error'] = TRUE;
          drupal_set_message('Het wachtwoord is niet correct', 'error');
        }
      }
    case 'submit_account_data':
      // Create new account.
      $user = array(
        'name' => $values['email'],
        'mail' => $values['email'],
        'pass' => $values['account_pass'],
        'status' => 0,
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
        // Save local access.
        $local_access = entity_create('local_access', array(
          'uid' => $account->uid,
          'campaign_id' => $campaign->id,
          'hash' => $hash,
        ));
        entity_save('local_access', $local_access);
        // Save caller data.
        $fields = array('first_name', 'middle_name', 'last_name');
        $name_parts = array();
        foreach ($fields as $field) {
          if (!empty($values[$field])) {
            $name_parts[$field] = $values[$field];
          }
        }
        $display_name = implode(' ', $name_parts);
        $caller = entity_create('caller', array(
          'caller_id' => $account->uid,
          'access_type' => 'local',
          'first_name' => $values['first_name'],
          'middle_name' => $values['middle_name'],
          'last_name' => $values['last_name'],
          'display_name' => $display_name,
          'phone' => $values['phone'],

        ));
        entity_save('caller', $caller);
        $form_state['finish'] = TRUE;
        $form_state['feedback'] = '<p>Er is een account aangemaakt, en een e-mail verzonden met instructies om je account te activeren.</p>';
      }
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
      $caller = entity_load_single('caller', $user->uid);
      $params = array(
        'name' => $caller->first_name,
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

function spbellen_custom_login_authenticate_validate($form, &$form_state) {
  if (!empty($form_state['values']['name'])) {
    // Do not allow any login from the current user's IP if the limit has been
    // reached. Default is 50 failed attempts allowed in one hour. This is
    // independent of the per-user limit to catch attempts from one IP to log
    // in to many different user accounts.  We have a reasonably high limit
    // since there may be only one apparent IP for all users at an institution.
    if (!flood_is_allowed('failed_login_attempt_ip', variable_get('user_failed_login_ip_limit', 50), variable_get('user_failed_login_ip_window', 3600))) {
      $form_state['flood_control_triggered'] = 'ip';
      return;
    }
    $account = db_query("SELECT * FROM {users} WHERE name = :name AND status = 1", array(':name' => $form_state['values']['name']))->fetchObject();
    if ($account) {
      if (variable_get('user_failed_login_identifier_uid_only', FALSE)) {
        // Register flood events based on the uid only, so they apply for any
        // IP address. This is the most secure option.
        $identifier = $account->uid;
      }
      else {
        // The default identifier is a combination of uid and IP address. This
        // is less secure but more resistant to denial-of-service attacks that
        // could lock out all users with public user names.
        $identifier = $account->uid . '-' . ip_address();
      }
      $form_state['flood_control_user_identifier'] = $identifier;

      // Don't allow login if the limit for this user has been reached.
      // Default is to allow 5 failed attempts every 6 hours.
      if (!flood_is_allowed('failed_login_attempt_user', variable_get('user_failed_login_user_limit', 5), variable_get('user_failed_login_user_window', 21600), $identifier)) {
        $form_state['flood_control_triggered'] = 'user';
        return;
      }
    }
  }
}

function spbellen_custom_login_final_validate($form, &$form_state) {
  // Always register an IP-based failed login event.
  flood_register_event('failed_login_attempt_ip', variable_get('user_failed_login_ip_window', 3600));
  // Register a per-user failed login event.
  if (isset($form_state['flood_control_user_identifier'])) {
    flood_register_event('failed_login_attempt_user', variable_get('user_failed_login_user_window', 21600), $form_state['flood_control_user_identifier']);
  }

  if (isset($form_state['flood_control_triggered'])) {
    if ($form_state['flood_control_triggered'] == 'user') {
      form_set_error('name', format_plural(variable_get('user_failed_login_user_limit', 5), 'Sorry, there has been more than one failed login attempt for this account. It is temporarily blocked. Try again later or <a href="@url">request a new password</a>.', 'Sorry, there have been more than @count failed login attempts for this account. It is temporarily blocked. Try again later or <a href="@url">request a new password</a>.', array('@url' => url('user/password'))));
    }
    else {
      // We did not find a uid, so the limit is IP-based.
      form_set_error('name', t('Sorry, too many failed login attempts from your IP address. This IP address is temporarily blocked. Try again later or <a href="@url">request a new password</a>.', array('@url' => url('user/password'))));
    }
  }
}
