<?php

/**
 * Import form definition and processing.
 */

/**
 * Import form definition.
 */
function spbellen_import_form($form, &$form_state) {
  if (empty($form_state['storage'])) {
    $form_state['storage'] = array();
  }
  $storage = &$form_state['storage'];
  $campaigns_list = spbellen_get_campaigns_list();

  $form['#prefix'] = '<div id="spbellen_import_ajax_form">';
  $form['#suffix'] = '</div>';

  $importer_info = spbellen_importer_info();
  if (empty($importer_info)) {
    $form['no_importers'] = array(
      '#markup' => '<p>Er zijn nog geen importers geïnstalleerd.</p>',
    );
  }
  else {
    if (empty($campaigns_list)) {
      $form['no_campaigns'] = array(
        '#markup' => '<p>Er zijn nog geen campagnes aangemaakt.</p>',
      );
    }
    else {
      if (empty($storage['campaign'])) {
        // Select campaign.
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
        $form['selected_campaign'] = array(
          '#markup' => '<p>' . $campaigns_list[$storage['campaign']],
        );
        $system_list = system_list('module_enabled');
        foreach ($importer_info as $module => $info) {
          $importer_list[$module] = $system_list[$module]->info['name'];
        }
        if (empty($storage['selected_importer']) && count($importer_info) > 1) {
          $form['select_importer'] = array(
            '#type' => 'select',
            '#title' => 'Kies een importer',
            '#name' => 'select_importer',
            '#options' => $importer_list,
            '#empty_option' => 'Kies:',
            '#ajax' => array(
              'wrapper' => 'spbellen_import_ajax_form',
              'callback' => 'spbellen_import_form_ajax_callback',
            ),
          );
        }
        else {
          if (empty($storage['selected_importer'])) {
            reset($importer_info);
            $storage['selected_importer'] = key($importer_info);
          }
          $form['selected_importer'] = array(
            '#markup' => '<p>' . $importer_list[$storage['selected_importer']],
          );
          $importer_info[$storage['selected_importer']]['form_definition_function']($form, $form_state);

          $form['import_submit'] = array(
            '#type' => 'submit',
            '#name' => 'import_submit',
            '#value' => 'Importeer',
          );
        }
      }
    }
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

      case 'select_importer':
        // Store selected campaign.
        if (!empty($values['select_importer'])) {
          $storage['selected_importer'] = $values['select_importer'];
        }
        break;
    }
  }

  // Add plugin validation.
  if (!empty($storage['selected_importer'])) {
    $importer_info = spbellen_importer_info();
    $function_name = $importer_info[$storage['selected_importer']]['form_validation_function'];
    $function_name($form, $form_state);
  }
}

/**
 * Import form submit functionality.
 */
function spbellen_import_form_submit($form, &$form_state) {
  $storage = &$form_state['storage'];
  if (!empty($storage['campaign'])) {
    // Execute importer submit functions.
    if (!empty($storage['selected_importer'])) {
      $importer_info = spbellen_importer_info();
      $function_name = $importer_info[$storage['selected_importer']]['form_submit_function'];
      $function_name($form, $form_state);
    }
  }
}

/**
 * Ajax callback.
 */
function spbellen_import_form_ajax_callback($form, &$form_state) {
  return $form;
}
