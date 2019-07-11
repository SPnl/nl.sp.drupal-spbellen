<?php

/**
 * @file
 * Campaign settings form.
 */

/**
 * Form definition.
 */
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
      else {
        if (!empty($storage['webform'])) {
          $webform = node_load($storage['webform']);
          if (!empty($webform->title)) {
            $form['webform_info'] = array(
              '#markup' => '<p><strong>Formulier<br/></strong> <a href="/node/' . $webform->nid . '/webform" target="_blank">' . check_plain($webform->title) . '</a></p>',
            );
          }
        }
      }

      $form['title'] = array(
        '#type' => 'textfield',
        '#title' => 'Titel',
        '#default_value' => empty($storage['title']) ? '' : $storage['title'],
      );

      $form['campaign_code'] = array(
        '#type' => 'textfield',
        '#title' => 'Campagne code',
        '#description' => 'Alleen gebruikers met deze code kunnen zich aanmelden, laat leeg indien iedereen zich moet kunnen aanmelden.',
        '#default_value' => empty($storage['campaign_code']) ? '' : $storage['campaign_code'],
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

      if ($storage['campaign'] !== 'new' && !empty($storage['campaign'])) {
        $group_info = spbellen_get_campaign_groups_info($storage['campaign']);
        if (!empty($group_info)) {
          $form['import_settings']['#tree'] = TRUE;
          foreach ($group_info as $group_id => $group_data) {
            if ($group_data['quantity'] > 0) {
              $form['import_settings'][$group_id] = array(
                'name' => array(
                  '#markup' => check_plain($group_data['title']),
                ),
                'date' => array(
                  '#markup' => check_plain(date('j-n-Y G:i:s', $group_data['date'])),
                ),
                'quantity' => array(
                  '#markup' => check_plain($group_data['quantity']),
                ),
                'active_import' => array(
                  '#type' => 'checkbox',
                  '#default_value' => $group_data['state'],
                ),
                'block_import' => array(
                  '#type' => 'checkbox',
                  '#default_value' => $group_data['block'],
                ),
                'weight' => array(
                  '#type' => 'weight',
                  '#title' => t('Weight'),
                  '#default_value' => $group_data['call_order'],
                  '#delta' => 10,
                  '#title_display' => 'invisible',
                ),
              );
            }
          }
        }
      }

      $form['other'] = array(
        '#markup' => '<strong>Overig</strong>',
      );

      $options = array(
        '0' => 'Toon eerst de contacten uit de afdeling of regio beller<ul><li>behalve als andere contacten minder vaak gebeld zijn</li><li>contacten in een import met een hoger gewicht gaan altijd voor</li></ul>',
        '1' => 'Toon alle contacten.',
        '2' => 'Toon alleen contacten uit afdeling beller.',
      );
      $form['afdeling_filter'] = array(
        '#type' => 'radios',
        '#name' => 'afdeling_filter',
        '#options' => $options,
        '#default_value' => empty($storage['afdeling_filter']) ? '0' : $storage['afdeling_filter'],
      );

      $form['disabled'] = array(
        '#type' => 'checkboxes',
        '#name' => 'disabled',
        '#options' => array('disabled' => 'Maak de campagne ontoegankelijk voor bellers'),
        '#default_value' => empty($storage['disabled']) ? array() : array('disabled'),
      );

      $form['send_fail_mail'] = array(
        '#type' => 'checkbox',
        '#title' => 'Stuur een e-mail als het contact niet bereikt kon worden',
        '#default_value' => empty($storage['send_fail_mail']) ? FALSE : $storage['send_fail_mail'],
      );

      $form['fail_mail'] = array(
        '#type' => 'fieldset',
        '#title' => 'Mail contact indien niet bereikt',
        '#states' => array(
          'visible' => array(
            ':input[name="send_fail_mail"]' => array('checked' => TRUE),
          ),
        ),
      );

      $form['fail_mail']['fail_mail_subject'] = array(
        '#type' => 'textfield',
        '#title' => 'Onderwerp',
        '#default_value' => empty($storage['fail_mail_subject']) ? '' : $storage['fail_mail_subject'],
      );

      $form['fail_mail']['tokens'] = array(
        '#markup' => 'Beschikbare tokens: [naam] en [telefoonnummer].',
      );

      $default = $storage['fail_mail_content'];
      $form['fail_mail']['fail_mail_content'] = array(
        '#type' => 'text_format',
        '#title' => 'Inhoud',
        '#format' => isset($default['format']) ? $default['format'] : 'filtered_html',
        '#default_value' => isset($default['value']) ? $default['value'] : 'Beste <naam>,',
      );

      $form['fail_mail']['fail_mail_from'] = array(
        '#type' => 'textfield',
        '#title' => 'Afzender e-mailadres',
        '#default_value' => empty($storage['fail_mail_from']) ? '' : $storage['fail_mail_from'],
      );

      $form['fail_mail']['fail_mail_to_test'] = array(
        '#type' => 'textfield',
        '#title' => 'Test e-mailadres',
      );

      $form['fail_mail']['send'] = array(
        '#type' => 'button',
        '#value' => 'Verstuur een testmail',
        '#name' => 'send',
        '#ajax' => array(
          'wrapper' => 'spbellen_ajax_form',
          'callback' => 'spbellen_form_ajax_callback',
        ),
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

/**
 * Form validation.
 */
function spbellen_campaign_form_validate($form, &$form_state) {
  $values = &$form_state['values'];
  $storage = &$form_state['storage'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
      case 'sync_targets':
        return;

      case 'send':
        if (!empty($values['send_fail_mail']) && !empty($values['fail_mail_to_test'])  && !empty($values['fail_mail_from']) && !empty($values['fail_mail_subject'])) {
          spbellen_send_test_mail($values);
        }
        else {
          form_set_error('fail_mail', 'Kon de test mail niet verzenden, er zijn lege velden in de mail!');
        }
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
            $storage['afdeling_filter'] = $campaign->afdeling_filter;
            $storage['disabled'] = $campaign->disabled;
            $storage['campaign_code'] = $campaign->campaign_code;
            $storage['campaign_start_page_text'] = $campaign->start_page_text;
            $storage['click2dial_call_number'] = $campaign->click2dial_call_number;
            $storage['fail_mail_subject'] = $campaign->fail_mail_subject;
            $storage['fail_mail_content'] = json_decode($campaign->fail_mail_content, TRUE);
            $storage['fail_mail_from'] = $campaign->fail_mail_from;
            $storage['send_fail_mail'] = $campaign->send_fail_mail;
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
        // Set import order.
        if (!empty($values['import_settings'])) {
          // Disable active when blocked.
          foreach ($values['import_settings'] as $group_id => $import_setting) {
            if ($import_setting['block_import'] == 1 && $import_setting['active_import'] == 1) {
              $values['import_settings'][$group_id]['active_import'] = 0;
            }
          }
        }
        // Store imort state.
        return;

      case 'delete':
        return;
    }
  }
  $form_state['rebuild'] = TRUE;
}

/**
 * Form submission.
 */
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
            $webform->path['alias'] = 'bellen/campagne/' . pathauto_cleanstring($webform->title);
            node_save($webform);
          }
        }
        // Store imort state.
        if (!empty($values['import_settings'])) {
          foreach ($values['import_settings'] as $import_settings_key => $import_settings_values) {
            db_update('spbellen_group_campaign_relation')
              ->fields(array(
                'state' => empty($import_settings_values['active_import']) ? 0 : 1,
                'call_order' => $import_settings_values['weight'],
                'block' => empty($import_settings_values['block_import']) ? 0 : 1,
              ))
              ->condition('group_id', $import_settings_key)
              ->execute();
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

/**
 * Implements theme_hook().
 */
function theme_spbellen_campaign_form($variables) {
  $form = $variables['form'];
  // Build form.
  if (!empty($form['import_settings'])) {
    foreach (element_children($form['import_settings']) as $id) {
      $form['import_settings'][$id]['weight']['#attributes']['class'] = array('import-setting-weight');
      $rows[] = array(
        'data' => array(
          drupal_render($form['import_settings'][$id]['name']),
          drupal_render($form['import_settings'][$id]['date']),
          drupal_render($form['import_settings'][$id]['quantity']),
          drupal_render($form['import_settings'][$id]['active_import']),
          drupal_render($form['import_settings'][$id]['block_import']),
          drupal_render($form['import_settings'][$id]['weight']),
        ),
        'class' => array('draggable'),
      );
    }
    $header = array(
      'Import',
      'Datum',
      'Aantal',
      'Actief',
      'Blokkeren',
      'Gewicht',
    );
    $table_id = 'spbellen-import-settings-tabledrag';
    $import_settings_table = theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array('id' => $table_id),
    ));
  }
  // Render form.
  $children_keys = element_children($form);
  $output = '';
  foreach ($children_keys as $key) {
    if (!empty($form[$key])) {
      $output .= drupal_render($form[$key]);
      if ($key === 'import_settings') {
        $output .= '<div class="form-item"><label>Belprioriteit</label>';
        $output .= $import_settings_table;
        $output .= '<div class="description"><ul><li>Pas het gewicht van imports aan om de belprioriteit te wijzigen. Contacten van een import met een lager gewicht worden eerst gebeld, ook al zijn er contacten in andere imports die minder vaak gebeld zijn. Geef imports een gelijk gewicht als je wilt dat contacten uit die imports die minder vaak gebeld zijn altijd prio krijgen boven contacten die vaker gebeld zijn.</li><li>Contacen in een niet actieve import worden niet gebeld, behalve als ze ook nog in een andere actieve import zitten.</li><li>Contacten in een geblokkeerde import worden niet gebeld, ook niet als ze ook nog in een andere actieve import zitten.</li></ul></div></div>';
      }
    }
  }
  return $output;
}

/**
 * Store campaign data.
 */
function spbellen_store_campaign($storage, $values) {
  if ($storage['campaign'] === 'new') {
    // Create campaign.
    $properties = array(
      'webform_id' => $values['webform'],
      'disabled' => empty($values['disabled']['disabled']) ? 0 : 1,
      'afdeling_filter' => $values['afdeling_filter'],
      'campaign_code' => $values['campaign_code'],
      'intro_text' => $values['intro_text'],
      'title' => $values['title'],
      'start_page_text' => !empty($values['campaign_intro_text']) ? $values['campaign_intro_text'] : '',
      'click2dial_call_number' => !empty($values['click2dial_call_number']) ? $values['click2dial_call_number'] : '',
      'fail_mail_subject' => !empty($values['fail_mail_subject']) ? $values['fail_mail_subject'] : '',
      'fail_mail_content' => !empty($values['fail_mail_content']) ? json_encode($values['fail_mail_content']) : '',
      'fail_mail_from' => !empty($values['fail_mail_from']) ? $values['fail_mail_from'] : '',
      'send_fail_mail' => !empty($values['send_fail_mail']) ? 1 : 0,
    );
    $campaign = entity_create('campaign', $properties);
  }
  else {
    $campaign = entity_load_single('campaign', $storage['campaign']);
    // Delete existing local access when campaign code is changed.
    if ($campaign->campaign_code !== $values['campaign_code']) {
      db_delete('spbellen_local_access')
        ->condition('campaign_id', $campaign->id)
        ->execute();
      drupal_set_message('Campagnecode is gewijzigd. Bestaande toegangsrechten van bellers zijn verwijderd.');
    }
    $campaign->webform_id = $storage['webform'];
    $campaign->disabled = empty($values['disabled']['disabled']) ? 0 : 1;
    $campaign->afdeling_filter = empty($values['afdeling_filter']) ? '0' : $values['afdeling_filter'];
    $campaign->campaign_code = empty($values['campaign_code']) ? '' : $values['campaign_code'];
    $campaign->title = empty($values['title']) ? '' : $values['title'];
    $campaign->start_page_text = empty($values['campaign_start_page_text']) ? '' : $values['campaign_start_page_text'];
    $campaign->click2dial_call_number = !empty($values['click2dial_call_number']) ? $values['click2dial_call_number'] : '';
    $campaign->fail_mail_subject = empty($values['fail_mail_subject']) ? '' : $values['fail_mail_subject'];
    $campaign->fail_mail_content = empty($values['fail_mail_content']) ? '' : json_encode($values['fail_mail_content']);
    $campaign->fail_mail_from = empty($values['fail_mail_from']) ? '' : $values['fail_mail_from'];
    $campaign->send_fail_mail = empty($values['send_fail_mail']) ? 0 : 1;
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

/**
 * Delete campaign.
 */
function spbellen_delete_campaign($campaign_id) {
  $result = entity_delete('campaign', $campaign_id);
  if ($result === FALSE) {
    drupal_set_message('Het verwijderen van de campagne is niet gelukt.', 'error');
  }
  else {
    drupal_set_message('Campagne is verwijderd.');
  }
}

/**
 * Send a test mail.
 */
function spbellen_send_test_mail($values) {
  drupal_mail('spbellen', 'fail_mail', $values['fail_mail_to_test'], language_default(), $values, $values['fail_mail_from']);
}
