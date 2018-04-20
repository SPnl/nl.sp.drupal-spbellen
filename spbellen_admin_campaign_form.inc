<?php

function spbellen_campaign_form($form, &$form_state) {
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
          'callback' => 'spbellen_form_ajax_callback',
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
        $form['webform'] = array(
          '#type' => 'select',
          '#title' => 'Kies het belscript webformulier',
          '#name' => 'webform',
          '#options' => $webforms_list,
          '#empty_option' => 'Kies:',
          '#required' => TRUE,
          '#default_value' => empty($storage['webform']) ? array() : $storage['webform'],
          '#ajax' => array(
            'wrapper' => 'spbellen_ajax_form',
            'callback' => 'spbellen_form_ajax_callback',
          ),
        );
      }

      $form['title'] = array(
        '#type' => 'textfield',
        '#title' => 'Titel',
        '#default_value' => empty($storage['title']) ? '' : $storage['title'],
      );

      $options = spbellen_click2dial_call_numbers_list();
      if (!empty($options)) {
        $form['click2dial_call_number'] = array(
          '#type' => 'select',
          '#title' => 'Selecteer het telefoonnummer waarmee gebeld gaat worden.',
          '#description' => 'Laat leeg om anoniem te bellen.',
          '#options' => $options,
          '#default_value' => empty($storage['click2dial_call_number']) ? '' : $storage['click2dial_call_number'],
          '#empty_option' => 'Kies:',
        );
      }

      $form['campaign_start_page_text'] = array(
        '#type' => 'textarea',
        '#title' => 'Campagne introductietekst',
        '#description' => 'Deze tekst wordt getoond na het inloggen, vóór het belscript wordt gestart.',
        '#format' => 'filtered_html',
        '#default_value' => empty($storage['campaign_start_page_text']) ? '' : $storage['campaign_start_page_text'],
      );

      if ($storage['campaign'] === 'new') {
        $form['intro_text'] = array(
          '#type' => 'textarea',
          '#title' => 'Hulptekst voor beller in het belscript',
          '#description' => 'Deze tekst wordt aan het begin van het belscript getoond, alvorens de vragen beginnen.',
          '#format' => 'filtered_html',
        );
      }

      $form['last_import_only'] = array(
        '#type' => 'checkboxes',
        '#title' => 'Bel alleen laatste import',
        '#name' => 'last_import_only',
        '#options' => array('enabled' => 'Bel alleen contacten van de laatste import'),
        '#default_value' => empty($storage['last_import_only']) ? array() : array('enabled'),
      );

      if ($storage['campaign'] !== 'new' && !empty($storage['campaign'])) {
        $options = spbellen_import_history_list($storage['campaign']);
        $defaults = spbellen_import_history_defaults($storage['campaign']);
        if (!empty($options)) {
          $form['active_imports'] = array(
            '#type' => 'checkboxes',
            '#title' => 'Selecteer welke contacten gebeld moeten worden',
            '#name' => 'active_imports',
            '#options' => $options,
            '#empty_option' => 'Kies:',
            '#default_value' => $defaults,
          );
        }
      }

      $form['afdeling_filter'] = array(
        '#type' => 'checkboxes',
        '#title' => 'Afdelingsfilter',
        '#description' => 'Contacten zonder locatieinformatie worden niet getoond!',
        '#name' => 'afdeling_filter',
        '#options' => array('enabled' => 'Toon alleen contacten uit afdeling beller'),
        '#default_value' => empty($storage['afdeling_filter']) ? array() : array('enabled'),
      );

      $form['disabled'] = array(
        '#type' => 'checkboxes',
        '#title' => 'Maak de campagne ontoegankelijk voor bellers',
        '#name' => 'disabled',
        '#options' => array('disabled' => 'Campagne uitgeschakeld'),
        '#default_value' => empty($storage['disabled']) ? array() : array('disabled'),
      );

      $form['campaign_code'] = array(
        '#type' => 'textfield',
        '#title' => 'Campagne code',
        '#description' => 'Alleen gebruikers met deze code kunnen zich aanmelden, laat leeg indien iedereen zich moet kunnen aanmelden.',
        '#default_value' => empty($storage['campaign_code']) ? '' : $storage['campaign_code'],
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
        '#markup' => 'Er is geen webformulier beschikbaar (<a href="/node/add/webform">voeg toe</a>).',
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
        $webform = entity_load_single('node', $campaign->webform_id);
        if (!empty($campaign)) {
          $storage['webform'] = $campaign->webform_id;
          $storage['last_import_only'] = $campaign->last_import_only;
          $storage['afdeling_filter'] = $campaign->afdeling_filter;
          $storage['disabled'] = $campaign->disabled;
          $storage['campaign_code'] = $campaign->campaign_code;
          //$storage['caller_check'] = $campaign->caller_check;
          $storage['campaign_start_page_text'] = $campaign->start_page_text;
          $storage['click2dial_call_number'] = $campaign->click2dial_call_number;
          if (!empty($campaign->title)) {
            $storage['title'] = $campaign->title;
          }
          else {
            $storage['title'] = $webform->title;
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
      // Validate title.
      if (empty($values['title'])) {
        form_set_error('title', 'Kies een titel voor de campaigne.');
      }
      return;
      break;
    case 'delete':
      return;
      break;
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
        spbellen_create_campaign_form($campaign);
      }
      else {
        if (!empty($campaign->title)) {
          // Save campaign form title.
          $webform = node_load($campaign->webform_id);
          $webform->title = $campaign->title;
          $webform->path['pathauto'] = 0;
          $webform->path['alias'] = 'bellen/campagne/'.pathauto_cleanstring($webform->title);
          node_save($webform);
        }
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

function spbellen_store_campaign($storage, $values) {
  if ($storage['campaign'] === 'new') {
    // Create campaign.
    $properties = array(
      'webform_id' => $values['webform'],
      'disabled' => empty($values['disabled']['disabled']) ? 0 : 1,
      'last_import_only' => empty($values['last_import_only']['enabled']) ? 0 : 1,
      'afdeling_filter' => empty($values['afdeling_filter']['enabled']) ? 0 : 1,
      'campaign_code' => $values['campaign_code'],
      'intro_text' => $values['intro_text'],
      'title' => $values['title'],
      'start_page_text' => !empty($values['campaign_intro_text']) ? $values['campaign_intro_text'] : '',
      'click2dial_call_number' => !empty($values['click2dial_call_number']) ? $values['click2dial_call_number'] : '',
    );
    $campaign = entity_create('campaign', $properties);
  }
  else {
    $campaign = entity_load_single('campaign', $storage['campaign']);
    $campaign->webform_id = $storage['webform'];
    $campaign->disabled = empty($values['disabled']['disabled']) ? 0 : 1;
    $campaign->last_import_only = empty($values['last_import_only']['enabled']) ? 0 : 1;
    $campaign->campaign_code = empty($values['campaign_code']) ? '' : $values['campaign_code'];
    $campaign->title = empty($values['title']) ? '' : $values['title'];
    $campaign->start_page_text = empty($values['campaign_start_page_text']) ? '' : $values['campaign_start_page_text'];
    $campaign->click2dial_call_number = !empty($values['click2dial_call_number']) ? $values['click2dial_call_number'] : '';
  }
  $result = entity_save('campaign', $campaign);

  if ($result !== FALSE) {
    drupal_set_message('Belcampagne is opgeslagen.');
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
