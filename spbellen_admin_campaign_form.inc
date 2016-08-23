<?php

function spbellen_campaign_form($form, &$form_state) {
  $form_state['groups_all'] = spbellen_get_all_civi_groups();
  $form_state['groups_all_but_smart'] = spbellen_get_all_but_smart_civi_groups();
  $campaigns_list = spbellen_get_campaigns_list();
  $webforms_list = spbellen_get_webforms_list();
  $storage = !empty($form_state['storage']) ? $form_state['storage'] : array();
  $form = array();

  $form['#prefix'] = '<div id="spbellen_ajax_form">';
  $form['#suffix'] = '</div>';

  if (empty($storage['campaign'])) {
    if (!empty($campaigns_list)) {
      // Select campaign.
      // ----------------------------------------------------------
      $form['select_campaign'] = array(
        '#type' => 'select',
        '#title' => 'Kies een bestaande belcampagne, of maak een nieuwe campagne aan.',
        '#name' => 'select_campaign',
        '#options' => $campaigns_list,
        '#empty_option' => 'Kies:',
        '#ajax' => array(
          'wrapper' => 'spbellen_ajax_form',
          'callback' => 'spbellen_campaign_form_ajax_callback',
        ),
      );
    }
    else {
      $form['no_campaigns'] = array(
        '#markup' => '<p>Er zijn nog geen campagnes aangemaakt.</p>',
      );
    }
    // Add campaign.
    $form['new_campaign'] = array(
      '#type' => 'button',
      '#value' => 'Nieuwe campagne',
      '#name' => 'new_campaign',
    );
  }
  else {
    if (!empty($webforms_list)) {
      // Create webform select element.
      if ($storage['campaign'] === 'new') {
        // Filter already set forms.
        $campaigns = entity_load('campaign');
        foreach ($campaigns as $campaign) {
          unset($webforms_list[$campaign->webform_id]);
        }
      }
      $disabled = ($storage['campaign'] === 'new') ? FALSE : TRUE;
      $form['webform'] = array(
        '#type' => 'select',
        '#title' => 'Kies het belscript webformulier',
        '#name' => 'webform',
        '#options' => $webforms_list,
        '#empty_option' => 'Kies:',
        '#required' => TRUE,
        '#default_value' => empty($storage['webform']) ? array() : $storage['webform'],
        '#disabled' => $disabled,
        '#ajax' => array(
          'wrapper' => 'spbellen_ajax_form',
          'callback' => 'spbellen_campaign_form_ajax_callback',
        ),
      );

      // Show message.
      $form['message_cache'] = array(
        '#markup' => '<p><strong>Leeg de Drupal cache als een CiviCRM groep niet te vinden is!</strong></p>',
      );

      $group_types = array(
        'call_group' => 'belgroep',
        'target_group' => 'doelgroep',
      );
      $filters = array(
        'call_group' => 'all',
        'target_group' => 'all_but_smart',
      );

      foreach ($group_types as $type_key => $type_name) {
        $form[$type_key.'_select'] = array(
          '#type' => 'fieldset',
          '#title' => 'Selecteer de '.$type_name.'en',
        );
        // Show stored groups.
        if (!empty($storage[$type_key.'s'])) {
          $markup = '<p><strong>Geselecteerde '.$type_name.'en:</strong><br/>';
          $form[$type_key.'_select']['selected_'.$type_key.'s'] = array(
            '#markup' => $markup,
          );
          foreach ($storage[$type_key.'s'] as $group_key => $group_name) {
            $form[$type_key.'_select']['del_'.$type_key.'_'.$group_key] = array(
              '#type' => 'button',
              '#name' => 'del_'.$type_key.'_'.$group_key,
              '#value' => 'x',
              '#suffix' => ' '.$group_name.'<br/>',
              '#ajax' => array(
                'wrapper' => 'spbellen_ajax_form',
                'callback' => 'spbellen_campaign_form_ajax_callback',
                '#default_value' => empty($storage[$type_key.'s']) ? array() : $storage[$type_key.'s'],
              ),
            );
          }
        }
        $form[$type_key.'_select'][$type_key] = array(
          '#type' => 'textfield',
          '#title' => 'Selecteer een '.$type_name,
          '#autocomplete_path' => 'spbellen/group/autocomplete/' . $filters[$type_key],
        );
        if ($type_key == 'target_group') {
          $form[$type_key.'_select'][$type_key]['#description'] = 'Geen selectie van smart groepen mogelijk.'; 
        }

        $form[$type_key.'_select']['add_'.$type_key] = array(
          '#type' => 'button',
          '#value' => 'Voeg toe',
          '#name' => 'add_'.$type_key,
          '#ajax' => array(
            'wrapper' => 'spbellen_ajax_form',
            'callback' => 'spbellen_campaign_form_ajax_callback',
          ),
        );
      }

      if ($storage['campaign'] !== 'new') {
        $form['sync'] = array(
          '#type' => 'fieldset',
          '#title' => 'Synchronizeer de doelgroep met CiviCRM',
        );
        $form['sync']['sync_targets'] = array(
          '#type' => 'submit',
          '#name' => 'sync_targets',
          '#value' => 'Sync',
          '#prefix' => '<p>Indien groepen gewijzigd zijn, eerst opslaan.</p>',
        );
      }

      if ($storage['campaign'] === 'new') {
        $form['intro_text'] = array(
          '#type' => 'textarea',
          '#title' => 'Introductietekst voor beller',
          '#format' => 'filtered_html',
        );
      }

      $form['disabled'] = array(
        '#type' => 'checkboxes',
        '#title' => 'Maak de campagne ontoegankelijk voor bellers',
        '#name' => 'disabled',
        '#options' => array('disabled' => 'Campagne uitgeschakeld'),
        '#default_value' => empty($storage['disabled']) ? array() : array('disabled'),
      );

      $form['open_access'] = array(
        '#type' => 'checkboxes',
        '#title' => 'Sta open aanmelding bellers toe',
        '#name' => 'open_access',
        '#options' => array('enabled' => 'Open aanmelding'),
        '#default_value' => empty($storage['open_access']) ? array() : array('enabled'),
      );

      $form['campaign_code'] = array(
        '#type' => 'textfield',
        '#title' => 'Campagne code',
        '#description' => 'Alleen gebruikers met deze code kunnen zich aanmelden, laat leeg indien iedereen zich moet kunnen aanmelden.',
        '#default_value' => empty($storage['campaign_code']) ? '' : $storage['campaign_code'],
        '#states' => array(
          'visible' => array(
            ':input[name="open_access[enabled]"]' => array('checked' => TRUE),
          ),
        ),
      );

      $form['caller_check'] = array(
        '#type' => 'checkboxes',
        '#title' => 'Bellers controleren',
        '#description' => 'Bellers worden dienen gecontroleerd te worden alvorens ze met contacten kunnen bellen.',
        '#name' => 'caller_check',
        '#options' => array('enabled' => 'Bellers controleren'),
        '#default_value' => empty($storage['caller_check']) ? array() : array('enabled'),
      );

      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Sla op',
        '#name' => 'submit',
      );

      if ($storage['campaign'] !== 'new') {
        $form['delete'] = array(
          '#type' => 'submit',
          '#value' => 'Verwijder',
          '#name' => 'delete',
        );
      }
    }
    else {
      $form['no_webforms'] = array(
        '#markup' => 'Er is geen webformulier beschikbaar (<a href="/node/webform/add">voeg toe</a>).',
      ); 
    }
  }
  return $form;
}

function spbellen_campaign_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $storage = &$form_state['storage'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'sync_targets':
      return;
      break;
    case 'select_campaign':
      // Store selected campaign.
      if (!empty($values['select_campaign'])) {
        $storage['campaign'] = $values['select_campaign'];
        // Preset stored campaign values.
        $campaign = entity_load_single('campaign', $storage['campaign']);
        if (!empty($campaign)) {
          $storage['webform'] = $campaign->webform_id;
          $storage['disabled'] = $campaign->disabled;
          $storage['open_access'] = $campaign->open_access;
          $storage['campaign_code'] = $campaign->campaign_code;
          $storage['caller_check'] = $campaign->caller_check;
          $campaign_groups = array();
          $query = new EntityFieldQuery();

          $result = $query
            ->entityCondition('entity_type', 'campaign_groups')
            ->propertyCondition('campaign_id', $storage['campaign']) 
            ->execute();

          if (!empty($result['campaign_groups'])) {
            $cgids = array_keys($result['campaign_groups']);
            $campaign_groups = entity_load('campaign_groups', $cgids);
          }
          foreach ($campaign_groups as $campaign_group) {
            $storage[$campaign_group->group_type.'s'][$campaign_group->civi_group_id] = $campaign_group->group_name;
          }
        }
      }
      break;
    case 'webform':
      $storage['webform'] = $values['webform'];
    case 'new_campaign':
      $storage['campaign'] = 'new';
      break;
    case 'submit':
      // Validate groups before submission.
      $types = array(
        'call_group' => 'belgroep',
        'target_group' => 'doelgroep',
      );
      foreach ($types as $type_key => $type_name) {
        if (empty($storage[$type_key.'s'])) {
          form_set_error($type_key, 'Selecteer minimaal één '.$type_name.'.');
        }
      }
      return;
      break;
    case 'delete':
      return;
      break;
    }
    // Add buttons.
    preg_match('/^add_(call_group|target_group)$/', $trigger, $matches);
    if (!empty($matches[1])) {
      $groups = $form_state['groups_all'];
      $group_raw = $values[$matches[1]];
      $group = spbellen_parse_id_form_select_value($group_raw);
      $storage[$matches[1].'s'][$group] = $groups[$group];
      $form_state['input'][$matches[1]] = '';
    }
    // Delete buttons.
    preg_match('/^del_(call_group|target_group)_([0-9]+)$/', $trigger, $matches);
    if (!empty($matches[2])) {
      unset($storage[$matches[1].'s'][$matches[2]]);
    }
  }
  $form_state['rebuild'] = TRUE;
}

function spbellen_campaign_form_submit($form, &$form_state) {
  $storage = $form_state['storage'];
  $values = $form_state['values'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'sync_targets':
      $campaign = entity_load_single('campaign', $storage['campaign']);
      if (!empty($campaign)) {
        /*
        $result = spbellen_get_targets($campaign);
        if ($result !== FALSE) {
          drupal_set_message('Doelgroep is gesynchronizeerd.');
        }
        else {
          drupal_set_message('Doelgroep kon niet worden gesynchronizeerd.', 'error');
        }
        */
        // Create operation for each CiviCRM target group for the campaign.
        //
        // Get sync_id.
        $query = "SELECT MAX(sync_id) AS old_sync_id FROM spbellen_targets WHERE campaign_id = :campaign_id";
        $sync_id = (db_query($query, array(':campaign_id' => $campaign->id))->fetchField() + 1);

        // Get campaign target groups.
        $all_but_smart_groups = spbellen_get_all_but_smart_civi_groups();
        $query = new EntityFieldQuery();
        $result = $query
          ->entityCondition('entity_type', 'campaign_groups')
          ->propertyCondition('campaign_id', $campaign->id)
          ->propertyCondition('group_type', 'target_group')
          ->execute();
        if (isset($result['campaign_groups'])) {
          $operations = array();
          $campaign_groups_ids = array_keys($result['campaign_groups']);
          $campaign_groups = entity_load('campaign_groups', $campaign_groups_ids);
          foreach ($campaign_groups as $campaign_group) {
            if (empty($all_but_smart_groups[$campaign_group->civi_group_id])) {
              drupal_set_message('Kan niet gesynced worden, smart group aanwezig.', 'error');
              return;
            }
            // Add operation for each campaign group.
            $operations[] = array(
              'spbellen_get_targets_batch',
              array($campaign->id, $campaign_group, $sync_id),
            );
          }
          $batch = array(
            'operations' => $operations,
            'finished' => 'spbellen_get_targets_batch_finished',
            'title' => 'Ophalen contacten',
            'init_message' => '0',
            'progress_message' => 'Groep @current van @total',
            'error_message' => 'Er is een fout opgetreden bij het ophalen van contacten',
          );
          batch_set($batch);
        }
      }
      return;
      break;
    case 'submit':
      // Store campaign in database.
      $campaign = spbellen_store_campaign($storage, $values);
      // Add spbellen intro form to campaign form.
      if ($storage['campaign'] === 'new') {
        spbellen_create_campaign_form($campaign);
      }
      cache_clear_all('spbellen_forms', 'cache');
      menu_rebuild();
      break;
    case 'delete':
      // Delete campaign.
      spbellen_delete_campaign($storage['campaign']);
      break;
    }
    // Clear spbellen_forms cache.
    cache_clear_all('spbellen_forms', 'cache');
  }
}

function spbellen_campaign_form_ajax_callback($form, &$form_state) {
  return $form;
}

function spbellen_store_campaign_groups($storage, $campaign) {
  $types = array(
    'call_group' => 'belgroep',
    'target_group' => 'doelgroep',
  );
  foreach ($types as $type_key => $type_name) {
    // Update and create campaign_groups.
    if (!empty($storage[$type_key.'s'])) {
      // Get stored groups.
      $stored_groups_ids = array();
      $stored_campaign_groups = spbellen_get_stored_campaign_groups($campaign->id, $type_key);
      foreach ($stored_campaign_groups as $stored_group) {
        $stored_groups_ids[$stored_group->civi_group_id] = $stored_group->id;
        $stored_groups_names[$stored_group->civi_group_id] = $stored_group->group_name;
      }
      $selected_groups = $storage[$type_key.'s']; 
      $added_groups = array_diff_assoc(array_keys($selected_groups),array_keys($stored_groups_ids));
      $removed_groups = array_diff_assoc(array_keys($stored_groups_ids), array_keys($selected_groups));

      foreach($added_groups as $group_id) {
        $properties = array(
          'civi_group_id' => $group_id,
          'campaign_id' => $campaign->id,
          'group_name' => $selected_groups[$group_id],
          'group_type' => $type_key,
          'disabled' => 0,
        );
        $campaign_groups = entity_create('campaign_groups', $properties);
        $result = $campaign_groups->save();
        if ($result === FALSE) {
          drupal_set_message('Het toevoegen van de campagnegroep "'.$selected_groups[$group_id].'" is niet gelukt.', 'error');
        }
      }
      foreach ($removed_groups as $group_id) {
        $campaign_group_id = $stored_groups_ids[$group_id];
        $result = entity_delete('campaign_groups', $campaign_group_id);
        if ($result === FALSE) {
          drupal_set_message('Het verwijderen van de campagnegroep "'.$stored_groups_names[$group_id].'" is niet gelukt.', 'error');
        }
        else {
          drupal_set_message('Campagnegroep "'.$stored_groups_names[$group_id].'" is verwijderd.');
        }
      }
    }
  }
}

function spbellen_store_campaign($storage, $values) {
  if ($storage['campaign'] === 'new') {
    // Create campaign.
    $properties = array(
      'webform_id' => $values['webform'],
      'disabled' => $values['disabled']['disabled'],
      'open_access' => $values['open_access']['enabled'],
      'caller_check' => $values['caller_check']['enabled'],
      'campaign_code' => $values['campaign_code'],
      'intro_text' => $values['intro_text'],
    );
    $campaign = entity_create('campaign', $properties);
  }
  else {
    $campaign = entity_load_single('campaign', $storage['campaign']);
    $campaign->webform_id = $values['webform'];
    $campaign->disabled = empty($values['disabled']['disabled']) ? 0 : 1;
    $campaign->open_access = empty($values['open_access']['enabled']) ? 0 : 1;
    $campaign->caller_check = empty($values['caller_check']['enabled']) ? 0 : 1;
    $campaign->campaign_code = empty($values['campaign_code']) ? '' : $values['campaign_code'];
  }
  $result = entity_save('campaign', $campaign);

  if ($result !== FALSE) {
    drupal_set_message('Belcampagne is opgeslagen.');

    // Store campaign groups.
    spbellen_store_campaign_groups($storage, $campaign);

    // Add spbellen intro webform to campaign webform.
  }
  else {
    drupal_set_message('Het opslaan van de campagne is niet gelukt.', 'error');
  }
  return $campaign;
}

function spbellen_delete_campaign($campaign_id) {
  $result = entity_delete('campaign', $campaign_id);
  if ($result === FALSE) {
    drupal_set_message('Het verwijderen van de campagne is niet gelukt.', 'error');
  }
  else {
    drupal_set_message('Campagne is verwijderd.');
  }
}
