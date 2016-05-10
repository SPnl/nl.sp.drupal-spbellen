<?php

function spbellen_campaign_form($form, &$form_state) {
  $groups = spbellen_get_civi_groups();
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
      // Filter introform.
      $introform_id = variable_get('spbellen_intro_webform', FALSE);
      if (!empty($introform_id)) {
        unset($webforms_list[$introform_id]);
      }
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

      // Add group selections.
      $form_state['groups'] = $groups;

      $group_types = array(
        'call_group' => 'belgroep',
        'target_group' => 'doelgroep',
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
          '#autocomplete_path' => 'spbellen/group/autocomplete',
        );

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

      if (!empty($storage['webform'])) {
        // Select the field value condition to show campaign questions.
        // --------------------------------------------------------------------
        $intro_form_id = variable_get('spbellen_intro_webform', FALSE);
        $intro_form_components = spbellen_get_webform_allowed_components($intro_form_id, array('select' => 'select'));
        if (!empty($intro_form_components)) {
          $intro_form = node_load($intro_form_id);
          $intro_form_component_select_options = webform_component_list($intro_form, $intro_form_components, FALSE);
          $form['condition'] = array(
            '#type' => 'fieldset',
            '#title' => 'Kies de conditie die bepaald of campagnevragen gesteld moeten worden.',
          );
          $form['condition']['condition_component'] = array(
            '#type' => 'select',
            '#title' => 'Formulier veld',
            '#options' => $intro_form_component_select_options,
            '#empty_option' => '- Geen -',
            '#ajax' => array(
              'wrapper' => 'spbellen_ajax_form',
              'callback' => 'spbellen_campaign_form_ajax_callback',
            ),
            '#name' => 'condition_component',
            '#default_value' => empty($storage['condition_component']) ? array() : $storage['condition_component'],
          );
          if (!empty($storage['condition_component'])) {
            $selected_component = $storage['condition_component'];
            $condition_accept_value_options = spbellen_parse_webform_select_options($intro_form, $selected_component);
            $form['condition']['condition_accept_value'] = array(
              '#type' => 'select',
              '#title' => 'Bij formulier veld waarde',
              '#options' => $condition_accept_value_options,
              '#empty_option' => '- Geen -',
              '#ajax' => array(
                'wrapper' => 'spbellen_ajax_form',
                'callback' => 'spbellen_campaign_form_ajax_callback',
              ),
              '#name' => 'condition_accept_value',
              '#default_value' => empty($storage['condition_accept_value']) ? array() : $storage['condition_accept_value'],
            );
          }
        }
        else {
          $form['condition']['condition_no_components'] = array(
            '#markup' => '<p>Er zijn geen geschikte webformulier velden gevonden.</p>',
          );
        }
      }

      if ($storage['campaign'] !== 'new') {
        $form['sync'] = array(
          '#type' => 'fieldset',
          '#title' => 'Synchronizeer de doelgroep met CiviCRM',
        );
        $form['sync']['sync_targets'] = array(
          '#type' => 'button',
          '#name' => 'sync_targets',
          '#value' => 'Sync',
          '#prefix' => '<p>Indien groepen gewijzigd zijn, eerst opslaan.</p>',
        );
      }

      $form['disabled'] = array(
        '#type' => 'checkboxes',
        '#title' => 'Maak de campagne ontoegankelijk voor bellers',
        '#name' => 'disabled',
        '#options' => array('disabled' => 'Campagne uitgeschakeld'),
        '#default_value' => empty($storage['disabled']) ? array() : array('disabled'),
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
      $campaign = entity_load_single('campaign', $storage['campaign']);
      if (!empty($campaign)) {
        $result = spbellen_get_targets($campaign, $disabled = 0);
        if ($result !== FALSE) {
          drupal_set_message('Doelgroep is gesynchronizeerd.');
        }
        else {
          drupal_set_message('Doelgroep kon niet worden gesynchronizeerd.', 'error');
        }
      }
      break;
    case 'condition_component':
      if (!empty($values['condition_component'])) {
        $storage['condition_component'] = $values['condition_component'];
      }
      break;
    case 'condition_accept_value':
      if (!empty($values['condition_accept_value'])) {
        $storage['condition_accept_value'] = $values['condition_accept_value'];
      }
      break;
    case 'select_campaign':
      // Store selected campaign.
      if (!empty($values['select_campaign'])) {
        $storage['campaign'] = $values['select_campaign'];
        // Preset stored campaign values.
        $campaign = entity_load_single('campaign', $storage['campaign']);
        if (!empty($campaign)) {
          $storage['webform'] = $campaign->webform_id;
          $storage['condition_component'] = $campaign->condition_component;
          $storage['condition_accept_value'] = $campaign->condition_accept_value;
          $storage['disabled'] = $campaign->disabled;
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
      // check if campaign questions condition is set.
      if (empty($values['condition_component']) || empty($values['condition_accept_value'])) {
        form_set_error('condition', 'Kies de conditie die bepaald of campagnevragen gesteld moeten worden.');
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
      $groups = spbellen_get_civi_groups();
      $group_raw = $values[$matches[1]];
      $group = spbellen_parse_group_form_select_value($group_raw);
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
    case 'submit':
      // Store campaign in database.
      $campaign = spbellen_store_campaign($storage, $values);
      // Add spbellen intro form to campaign form.
      if ($storage['campaign'] === 'new') {
        spbellen_add_intro_form($campaign);
      }
      break;
    case 'delete':
      // Delete campaign.
      spbellen_delete_campaign($storage);
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
      'condition_component' => $values['condition_component'],
      'condition_accept_value' => $values['condition_accept_value'],
      'disabled' => $values['disabled']['disabled'],
    );
    $campaign = entity_create('campaign', $properties);
  }
  else {
    $campaign = entity_load_single('campaign', $storage['campaign']);
    $campaign->webform_id = $values['webform'];
    $campaign->condition_component = $values['condition_component'];
    $campaign->condition_accept_value = $values['condition_accept_value'];
    $campaign->disabled = empty($values['disabled']['disabled']) ? 0 : 1;
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

function spbellen_delete_campaign($storage) {
  $campaign = entity_load_single('campaign', $storage['campaign']);
  if (!empty($campaign)) {
    $result = entity_delete('campaign', $campaign->id);
    if ($result === FALSE) {
      drupal_set_message('Het verwijderen van de campagne is niet gelukt.', 'error');
    }
    else {
      drupal_set_message('Campagne is verwijderd.');
    }
  }
}

