<?php

function spbellen_start_page_form($form, &$form_state) {
  $form = array();
  $intro = variable_get('spbellen_general_start_page', NULL);
  if (empty($form_state['general_start_page_visited']) && empty($_SESSION['general_start_page_visited']) && !empty($intro['value'])) {
    $form['intro'] = array(
      '#markup' => check_markup($intro['value'], $intro['format']),
    );
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => 'Verder',
      '#name' => 'submit_intro',
    );
  }
  else {
    // Select campaign page.
    $campaigns = spbellen_get_campaigns();
    $campaign_list = array();
    foreach ($campaigns as $campaign) {
      if (!empty($campaign->webform_id) && $campaign->disabled == 0) {
        $webform = entity_load_single('node', $campaign->webform_id);
        $campaign_list[$campaign->webform_id] = $webform->title;
      }
    }
    // If only one campaign, go there directly.
    if (count($campaign_list) === 1) {
      $webform_id = key($campaign_list);
      $alias = spbellen_get_campaign_form_alias($webform_id);
      $path = 'aanmelden/campagne/' . $alias;
      drupal_goto($path);
    }
    // Otherwise show selection form.
    if (!empty($campaign_list)) {
      $intro = variable_get('spbellen_campaign_select_intro', NULL);
      if (!empty($intro['value'])) {
        // Introduction.
        $form['spbellen_campaign_select_intro'] = array(
          '#markup' => check_markup($intro['value'], $intro['format']),
        );
      }

      $form['campaign'] = array(
        '#type' => 'select',
        '#title' => 'Selecteer een belcampagne:',
        '#options' => $campaign_list,
        '#required' => TRUE,
      );
      $form['submit'] = array(
        '#type' => 'submit',
        '#value' => 'Selecteer',
        '#name' => 'submit_select_campaign',
      );
    }
    else {
      $form['no_campaigns'] = array(
        '#markup' => '<p>Er is geen campagne beschikbaar.</p>',
      );
    }
  }
  return $form;
}

function spbellen_start_page_form_validate($form, &$form_state) {
  $values = $form_state['values'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'submit_intro':
      $form_state['general_start_page_visited'] = TRUE;
      $_SESSION['general_start_page_visited'] = TRUE;
      $form_state['rebuild'] = TRUE;
      break;
    }
  }
}

function spbellen_start_page_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'submit_select_campaign':
      $webform_id = $form_state['values']['campaign'];
      $alias = spbellen_get_campaign_form_alias($webform_id);
      $path = 'aanmelden/campagne/' . $alias;
      drupal_goto($path);
      break;
    }
  }
}


