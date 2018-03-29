<?php

/**
 * Import form definition and processing.
 */

/**
 * Import form definition.
 */
function spbellen_import_form($form, &$form_state) {
  $storage = !empty($form_state['storage']) ? $form_state['storage'] : array();
  $campaigns_list = spbellen_get_campaigns_list();

  $form['#prefix'] = '<div id="spbellen_import_ajax_form">';
  $form['#suffix'] = '</div>';

  if (!empty($campaigns_list)) {
    // Select campaign.
    // ----------------------------------------------------------
    $form['select_campaign'] = array(
      '#type' => 'select',
      '#title' => 'Kies een belcampagne.',
      '#name' => 'select_campaign',
      '#options' => $campaigns_list,
      '#empty_option' => 'Kies:',
      '#ajax' => array(
        'wrapper' => 'spbellen_import_ajax_form',
        'callback' => 'spbellen_import_form_ajax_callback',
      ),
    );
  }
  else {
    $form['no_campaigns'] = array(
      '#markup' => '<p>Er zijn nog geen campagnes aangemaakt.</p>',
    );
  }
  if (!empty($storage['campaign'])) {

    // Get import forms.
    $importer_info = spbellen_importer_info();
    if (!empty($importer_info)) {
      foreach ($importer_info as $info) {
        $info['form_definition_function']($form, $form_state);
      }
    }

    $form['import_submit'] = array(
      '#type' => 'submit',
      '#name' => 'import_submit',
      '#value' => 'Importeer',
    );
  }
  return $form;
}

/**
 * Import form validation.
 */
function spbellen_import_form_validate($form, &$form_state) {
  $values = $form_state['values'];
  $storage = &$form_state['storage'];

  if (!empty($form_state['triggering_element']['#name'])) {
    // Check fixed name buttons.
    $trigger = $form_state['triggering_element']['#name'];
    switch ($trigger) {
      case 'select_campaign':
        // Store selected campaign.
        if (!empty($values['select_campaign'])) {
          $storage['campaign'] = $values['select_campaign'];
        }
        break;

    }
  }

  // Add plugin validation.
  $importer_info = spbellen_importer_info();
  if (!empty($importer_info)) {
    foreach ($importer_info as $info) {
      $info['form_validation_function']($form, $form_state);
    }
  }
}

/**
 * Import form submit functionality.
 */
function spbellen_import_form_submit($form, &$form_state) {
  $storage = &$form_state['storage'];
  if (!empty($storage['campaign'])) {
    // Get sync id for importers.
    $campaign = entity_load_single('campaign', $storage['campaign']);
    if (!empty($campaign)) {
      $query = "SELECT MAX(sync_id) AS old_sync_id FROM spbellen_targets WHERE campaign_id = :campaign_id";
      $sync_id = (db_query($query, array(':campaign_id' => $campaign->id))->fetchField() + 1);
      $storage['sync_id'] = $sync_id;

      // Execute importer submit functions.
      $importer_info = spbellen_importer_info();
      if (!empty($importer_info)) {
        foreach ($importer_info as $info) {
          $info['form_submit_function']($form, $form_state);
        }
      }
    }
  }
}

function spbellen_import_form_ajax_callback($form, &$form_state) {
  return $form;
}
