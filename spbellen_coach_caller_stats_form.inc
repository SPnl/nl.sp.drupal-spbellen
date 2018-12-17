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

  if (!empty($values['campaign'])) {
    $campaign_id = $values['campaign'];

    $options = spbellen_get_campaign_callers($campaign_id);

    $form['caller'] = array(
      '#type' => 'select',
      '#title' => 'Beller',
      '#options' => $options,
      '#empty_option' => 'Kies:',
      '#ajax' => array(
        'wrapper' => 'spbellen_ajax_form',
        'callback' => 'spbellen_form_ajax_callback',
      ),
    );

    if (!empty($values['caller']) && !empty($options[$values['caller']])) {
      $caller_id = spbellen_parse_id_from_select_value($values['caller']);

      $call_dates = spbellen_get_call_dates($campaign_id, $caller_id);
      $form['call_date'] = array(
        '#title' => 'Datum',
        '#type' => 'select',
        '#options' => $call_dates,
        '#ajax' => array(
          'wrapper' => 'spbellen_ajax_form',
          'callback' => 'spbellen_form_ajax_callback',
        ),
      );

      if (empty($values['call_date'])) {
        $call_date = reset($call_dates);
      }
      else {
        $call_date = $values['call_date'];
      }
      $call_date = $values['call_date'];

      $general_stats_caller = spbellen_get_caller_general_stats($caller_id, NULL);
      $general_stats_campaign = spbellen_get_caller_general_stats(NULL, $campaign_id);
      $general_stats_caller_campaign = spbellen_get_caller_general_stats($caller_id, $campaign_id);
      $general_stats_campaign_date = spbellen_get_caller_general_stats(NULL, $campaign_id, $call_date);
      $general_stats_caller_campaign_date = spbellen_get_caller_general_stats($caller_id, $campaign_id, $call_date);
      $result_stats_campaign = spbellen_get_campaign_result_stats(NULL, $campaign_id);
      $result_stats_caller_campaign = spbellen_get_campaign_result_stats($caller_id, $campaign_id);
      $result_stats_campaign_date = spbellen_get_campaign_result_stats(NULL, $campaign_id, $call_date);
      $result_stats_caller_campaign_date = spbellen_get_campaign_result_stats($caller_id, $campaign_id, $call_date);
      $variables = array(
        'general_stats_caller' => $general_stats_caller,
        'general_stats_campaign' => $general_stats_campaign,
        'general_stats_caller_campaign' => $general_stats_caller_campaign,
        'general_stats_campaign_date' => $general_stats_campaign_date,
        'general_stats_caller_campaign_date' => $general_stats_caller_campaign_date,
        'result_stats_caller_campaign' => $result_stats_caller_campaign,
        'result_stats_campaign' => $result_stats_campaign,
        'result_stats_caller_campaign_date' => $result_stats_caller_campaign_date,
        'result_stats_campaign_date' => $result_stats_campaign_date,
      );
      $markup = theme('spbellen_caller_overview_theme', $variables);

      $form['general_stats'] = array(
        '#markup' => $markup,
      );
    }
  }
  return $form;
}
