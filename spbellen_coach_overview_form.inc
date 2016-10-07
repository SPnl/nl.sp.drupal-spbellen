<?php

function spbellen_coach_overview_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'spbellen', 'spbellen_coach_form');
  $values = !empty($form_state['values']) ? $form_state['values'] : array();
  $storage = !empty($form_state['storage']) ? $form_state['storage'] : array();

  $form = array();
  $form['#prefix'] = '<div id="spbellen_ajax_form">';
  $form['#suffix'] = '</div>';

  $campaigns_list = spbellen_get_campaigns_list();
  if (!empty($campaigns_list)) {
    // Select campaign.
    // ----------------------------------------------------------
    $form['select_campaign'] = array(
      '#type' => 'select',
      '#title' => 'Kies een belcampagne.',
      '#name' => 'select_campaign',
      '#options' => $campaigns_list,
      '#default_value' => empty($_SESSION['spbellen']['coach_selected_campaign']) ? array() : $_SESSION['spbellen']['coach_selected_campaign'],
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

  return $form;
}

function spbellen_coach_overview_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $storage = &$form_state['storage'];

  spbellen_coach_add_select_campaign_form_validate($form_state);

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
    case 'xxxx':
      break;
    }
  }
}
