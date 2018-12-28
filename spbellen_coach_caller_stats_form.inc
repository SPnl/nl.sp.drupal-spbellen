<?php

/**
 * @file
 * Caller review form.
 */

 /**
  * Caller review form definition.
  */
function spbellen_caller_stats_form($form, &$form_state) {
  $values = empty($form_state['values']) ? array() : $form_state['values'];

  $campaign = !empty($values['campaign']) ? $values['campaign'] : '';
  $call_date = !empty($values['call_date']) ? $values['call_date'] : '';
  $caller = !empty($values['caller']) ? $values['caller'] : '';

  $callers = spbellen_get_campaign_callers($campaign);

  // Reset choice dependent values on change.
  if (!empty($caller) && !empty($form_state['last_caller'])) {
    if ($form_state['last_caller'] != $caller) {
      unset($values['call_date']);
      unset($call_date);
      unset($form_state['input']['call_date']);
    }
  }
  if (!empty($campaign) && !empty($form_state['last_campaign'])) {
    if ($form_state['last_campaign'] != $campaign) {
      unset($values['call_date']);
      unset($call_date);
      unset($form_state['input']['call_date']);
      unset($values['caller']);
      unset($caller);
      unset($form_state['input']['caller']);
    }
  }

  // Get caller id.
  $caller_id = empty($caller) ? '' : spbellen_parse_id_from_select_value($caller);

  // Create form.
  $form = array();

  $form['#prefix'] = '<div id="spbellen_ajax_form">';
  $form['#suffix'] = '</div>';

  // Attach css.
  $form['#attached']['css'] = array(
    drupal_get_path('module', 'spbellen') . '/css/spbellen_caller_stats.css',
  );

  $campaign_list = spbellen_get_campaigns_list();
  $form['campaign'] = array(
    '#type' => 'select',
    '#title' => 'Campagne',
    '#options' => $campaign_list,
    '#empty_option' => 'Kies:',
    '#ajax' => array(
      'wrapper' => 'spbellen_ajax_form',
      'callback' => 'spbellen_form_ajax_callback',
    ),
  );

  if (!empty($campaign)) {

    $call_dates = spbellen_get_call_dates($campaign, $caller_id);
    $form['call_date'] = array(
      '#title' => 'Datum',
      '#type' => 'select',
      '#options' => $call_dates,
      '#empty_option' => 'Kies:',
      '#ajax' => array(
        'wrapper' => 'spbellen_ajax_form',
        'callback' => 'spbellen_form_ajax_callback',
      ),
    );

    $form['caller'] = array(
      '#type' => 'select',
      '#title' => 'Beller',
      '#options' => $callers,
      '#empty_option' => 'Kies:',
      '#ajax' => array(
        'wrapper' => 'spbellen_ajax_form',
        'callback' => 'spbellen_form_ajax_callback',
      ),
    );

    if (empty($caller)) {
      $variables = array(
        'general_stats' => spbellen_get_caller_stats('all', $campaign, $call_date),
        'result_stats' => spbellen_get_result_stats('all', $campaign, $call_date),
      );
      $markup = theme('spbellen_callers_overview_theme', $variables);
      $form['callers_stats'] = array(
        '#markup' => $markup,
      );
    }

    if (!empty($caller) && !empty($callers[$caller])) {

      $variables = array(
        'general_stats_caller' => spbellen_get_caller_stats($caller_id, NULL),
        'general_stats_campaign' => spbellen_get_caller_stats(NULL, $campaign),
        'general_stats_caller_campaign' => spbellen_get_caller_stats($caller_id, $campaign),
        'general_stats_campaign_date' => empty($call_date) ? '' : spbellen_get_caller_stats(NULL, $campaign, $call_date),
        'general_stats_caller_campaign_date' => empty($call_date) ? '' : spbellen_get_caller_stats($caller_id, $campaign, $call_date),
        'result_stats_caller_campaign' => spbellen_get_result_stats($caller_id, $campaign),
        'result_stats_campaign' => spbellen_get_result_stats(NULL, $campaign),
        'result_stats_caller_campaign_date' => empty($call_date) ? '' : spbellen_get_result_stats($caller_id, $campaign, $call_date),
        'result_stats_campaign_date' => empty($call_date) ? '' : spbellen_get_result_stats(NULL, $campaign, $call_date),
      );
      $markup = theme('spbellen_caller_overview_theme', $variables);

      $form['caller_stats'] = array(
        '#markup' => $markup,
      );
    }
  }
  if (!empty($campaign)) {
    $form_state['last_campaign'] = $campaign;
  }
  if (!empty($caller)) {
    $form_state['last_caller'] = $caller;
  }
  if (!empty($call_date)) {
    $form_state['last_call_date'] = $call_date;
  }
  return $form;
}
