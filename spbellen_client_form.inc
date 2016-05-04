<?php

function spbellen_client_form($form, &$form_state) {
  if (empty($form_state['states'])) {
    $form_state['states'][] = 'start';
  }

  $state = end($form_state['states']);

  $form['#prefix'] = '<div id="spbellen_ajax_form">';
  $form['#suffix'] = '</div>';

  switch ($state) {
  case 'start':
    $form['start'] = array(
      '#type' => 'radios',
      '#title' => 'Heeft u iemand gesproken?',
      '#options' => array(
        'call_answered' => 'Ja',
        'voicemail' => 'Nee, voicemail',
        'wrong_number' => 'Nee, verkeerd nummer',
      ),
    );
    break;
  case 'call_answered':
    $form['call_answered'] = array(
      '#type' => 'radios',
      '#title' => 'Is uw contact beschikbaar om uw vragen te beantwoorden?',
      '#options' => array(
        'answering_questions' => 'Ja',
        'call_back' => 'Nee, graag terugbellen',
        'blacklist' => 'Nee, niet meer bellen',
      ),
    );
    break;
  case 'answering_questions':
    $form['spbellen_answering_questions'] = array(
      '#type' => 'fieldset',
      '#title' => 'Vragen',
    );
    $webform = spbellen_get_webform();
    $form['spbellen_answering_questions']['webform'] = array(
      '#markup' => $webform,
    );
    break;
  case 'call_back':
    $form['call_back'] = array(
      '#markup' => '<p>To do: call_back processing.</p>',
    );
    break;
  case 'blacklist':
    $form['call_back'] = array(
      '#markup' => '<p>To do: blacklist processing.</p>',
    );
    break;
  case 'voicemail':
    $form['voicemail'] = array(
      '#type' => 'radios',
      '#title' => 'Heeft u de voicemail ingesproken?',
      '#options' => array(
        'voicemail_unanswered' => 'Nee',
        'voicemail_answered_call_back' => 'Ja, nog een keer bellen.',
        'voicemail_answered_no_call_back' => 'Ja, niet meer bellen.',
      ),
    );
    break;
  case 'voicemail_unanswered':
    $form['call_back'] = array(
      '#markup' => '<p>To do: voicemail_unanswered processing.</p>',
    );
    break;
  case 'voicemail_answered_call_back':
    $form['call_back'] = array(
      '#markup' => '<p>To do: voicemail_answered_call_back processing.</p>',
    );
    break;
  case 'voicemail_answered_no_call_back':
    $form['call_back'] = array(
      '#markup' => '<p>To do: voicemail_answered_no_call_back processing.</p>',
    );
    break;
  case 'wrong_number':
    $form['call_back'] = array(
      '#markup' => '<p>To do: wrong_number processing.</p>',
    );
    break;
  }

  // Add back button.
  if (! in_array($state, array('start', 'end'))) {
    $form['back'] = array(
      '#type' => 'button',
      '#value' => 'Terug',
      '#name' => 'back',
    );
  }

  foreach ($form as $key => $item) {
    if (!empty($item['#type'])) {
      if (in_array($item['#type'], array('button', 'radios'))) {
        $form[$key]['#ajax'] = array(
          'wrapper' => 'spbellen_ajax_form',
          'callback' => 'spbellen_client_form_ajax_callback',
        );
      }
    }
  }

  return $form;
}

function spbellen_client_form_ajax_callback($form, &$form_state) {
  return $form;
}

function spbellen_client_form_validate($form, &$form_state) {
  if (!empty($form_state['triggering_element']['#name'])) {
    switch ($form_state['triggering_element']['#name']) {
      case 'back':
      array_pop($form_state['states']);
      $form_state['rebuild'] = TRUE;
      return;
      break;
    }
  }
  if (!empty($form_state['states'])) {
    $state = end($form_state['states']);
    switch ($state) {
    case 'start':
      switch ($form_state['values']['start']) {
      case 'wrong_number':
        dpm('To do: process wrong_number.');
        break;
      }
      break;
    case 'call_answered':
      switch ($form_state['values']['call_answered']) {
      case 'call_back':
        dpm('To do: process callback.');
        break;
      case 'blacklist':
        dpm('To do: process blacklist.');
        break;
      }
      break;
    case 'voicemail':
      switch ($form_state['values']['voicemail']) {
      case 'voicemail_unanswered':
        dpm('To do: process voicemail_unanswered.');
        break;
      case 'voicemail_answered_call_back':
        dpm('To do: process voicemail_answered_call_back.');
        break;
      case 'voicemail_answered_no_call_back':
        dpm('To do: process blacklist.');
        break;
      }
      break;
    }
    $form_state['states'][] = $form_state['values'][$state];
    $form_state['rebuild'] = TRUE;
  }
}

function spbellen_get_webform() {
  $active_campaign_id = variable_get('spbellen_active_campaign', '');
  $campaign = entity_load_single('campaign', $active_campaign_id);
  $webform = node_load($campaign->webform_id);
  $form = drupal_get_form('webform_client_form_' . $webform->nid, $webform, array(), TRUE, FALSE);
  dpm($form);
  return render($form);

}

function spbellen_client_form_submit($form, &$form_state) {
}
