<?php

/**
 * @file
 * Helper function.
 */

/**
 * Get webforms.
 */
function spbellen_get_webforms() {
  $webforms = &drupal_static(__FUNCTION__);
  if (!isset($webforms)) {
    $webform_node_types = webform_variable_get('webform_node_types');
    $webforms = array();
    $query = new EntityFieldQuery();

    $result = $query
      ->entityCondition('entity_type', 'node')
      ->entityCondition('bundle', $webform_node_types, 'IN')
      ->execute();

    if (!empty($result['node'])) {
      $nids = array_keys($result['node']);
      $webforms = entity_load('node', $nids);
    }
  }
  return $webforms;
}

/**
 * Return list of webforms usable for form.
 */
function spbellen_get_webforms_list($type = NULL) {
  $webforms_list = array();
  $webforms = spbellen_get_webforms();
  if (!empty($webforms)) {
    foreach ($webforms as $webform) {
      if (empty($type)) {
        $webforms_list[$webform->nid] = $webform->title;
      }
      else {
        if ($webform->type == $type) {
          $webforms_list[$webform->nid] = $webform->title;
        }
      }
    }
  }
  return $webforms_list;
}

/**
 * Get campaign by webform id.
 */
function spbellen_get_campaign($webform_id) {
  $campaign_id = spbellen_get_campaign_id($webform_id);
  if (!empty($campaign_id)) {
    $campaign = entity_load_single('campaign', $campaign_id);
    if (!empty($campaign)) {
      return $campaign;
    }
  }
  return FALSE;
}

/**
 * Returns campaigns.
 */
function spbellen_get_campaigns() {
  $campaigns = &drupal_static(__FUNCTION__);
  if (!isset($campaigns)) {
    $campaigns = array();
    $query = new EntityFieldQuery();

    $result = $query
      ->entityCondition('entity_type', 'campaign')
      ->execute();

    if (!empty($result['campaign'])) {
      $cids = array_keys($result['campaign']);
      $campaigns = entity_load('campaign', $cids);
    }
    $campaigns = array_reverse($campaigns, TRUE);
  }
  return $campaigns;
}

/**
 * Returns list of campaigns, usable for forms.
 */
function spbellen_get_campaigns_list() {
  $campaigns_list = &drupal_static(__FUNCTION__);
  if (!isset($campaigns)) {
    $campaigns_list = array();
    $campaigns = spbellen_get_campaigns();
    if (!empty($campaigns)) {
      foreach ($campaigns as $campaign) {
        if (!empty($campaign->webform_id)) {
          $webform = entity_load_single('node', $campaign->webform_id);
          if (!empty($webform)) {
            $campaigns_list[$campaign->id] = $webform->title . ' (' . $campaign->id . ')';
          }
        }
      }
    }
  }
  return $campaigns_list;
}

/**
 * Returns contacts for autocomplete contact search form.
 */
function spbellen_contact_search_autocomplete($user_id, $webform_id, $string) {
  $matches = array();
  if (!empty($string)) {
    if (spbellen_campaign_form_access($webform_id)) {
      $campaign_id = spbellen_get_campaign_id($webform_id);
      if (!empty($campaign_id)) {
        $max_attempts = variable_get('spbellen_max_attempts', '5');
        $from_where = spbellen_get_from_where();
        $query = "SELECT ta.id, cd.phone, cd.display_name" . $from_where . "(cd.phone LIKE :phonenumber OR cd.display_name LIKE :display_name) AND (result = 'not_answered' OR result = 'answered_call_back' OR result = 'call_appointment' OR result = 'skipped' OR result IS NULL) AND (occupied = 0 OR (occupied = 1 AND (ta.date < :occupied_time OR ta.caller_id = :caller_id))) ORDER BY call_order ASC, attempts ASC, ta.date ASC, RAND() LIMIT 10";
        $substitutions = array(
          ':cid' => $campaign_id,
          ':phonenumber' => '%' . db_like($string) . '%',
          ':display_name' => '%' . db_like($string) . '%',
          ':caller_id' => $user_id,
          ':occupied_time' => time() - 1800,
          ':current_time' => time(),
          ':max_attempts' => $max_attempts,
        );
        $result = db_query($query, $substitutions);
        if ($result) {
          while ($row = $result->fetchAssoc()) {
            $text = $row['phone'] . ', ' . $row['display_name'] . ' (' . $row['id'] . ')';
            $matches[$text] = $text;
          }
        }
      }
    }
  }

  // Return the result to the form in json.
  drupal_json_output($matches);
}

/**
 * Helper function to parse id from select value.
 */
function spbellen_parse_id_from_select_value($id_raw) {
  $id = '';
  preg_match('@^.+\(([0-9]+)\)$@', $id_raw, $matches);
  if (!empty($matches[1])) {
    $id = $matches[1];
  }
  return $id;
}

/**
 * Create campaign form.
 */
function spbellen_create_campaign_form($campaign) {
  $error = FALSE;
  $campaign_webform = node_load($campaign->webform_id);
  if (!empty($campaign_webform->webform)) {
    $form = $campaign_webform->webform;
    $campaign_components = !empty($form['components']) ? $form['components'] : array();
    $campaign_conditionals = !empty($form['conditionals']) ? $form['conditionals'] : array();

    // Create new campaign webform.
    $joined_component_id = 1;
    $parent = 0;

    // Add test component to check for double submissions.
    $joined_components[$joined_component_id] = array(
      'pid' => $parent,
      'form_key' => 'valid_test',
      'name' => 'Valid test',
      'type' => 'textfield',
      'value' => '',
      'extra' => array(
        'private' => 0,
        'analysis' => FALSE,
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $joined_component_id++;

    // Add contact components.
    $contact_fields = spbellen_contact_fields();
    foreach ($contact_fields as $key => $name) {
      $joined_components[$joined_component_id] = array(
        'pid' => $parent,
        'form_key' => 'spbellen_hidden_' . $key,
        'name' => $name . ' (verborgen)',
        'type' => 'hidden',
        'value' => '',
        'extra' => array(
          'private' => 0,
          'hidden_type' => 'value',
          'analysis' => FALSE,
        ),
        'required' => 0,
        'weight' => $joined_component_id,
        'page_num' => 1,
      );
      $joined_component_id++;
    }

    // Add intro components.
    // Intro text.
    $joined_components[$joined_component_id] = array(
      'pid' => $parent,
      'form_key' => 'intro_text',
      'name' => 'Introductietekst',
      'type' => 'markup',
      'value' => $campaign->intro_text,
      'extra' => array(
        'format' => 'filtered_html',
        'private' => 0,
        'display_on' => 'form',
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $text_intro_component_id = $joined_component_id;
    $joined_component_id++;

    // Call answered?
    $joined_components[$joined_component_id] = array(
      'pid' => $parent,
      'form_key' => 'call_answered',
      'name' => 'Is de telefoon opgenomen?',
      'type' => 'select',
      'value' => '',
      'extra' => array(
        'items' => "answered|Ja\nnot_answered|Nee\nwrong_number|Nee, verkeerd nummer",
        'multiple' => 0,
        'title_display' => 'before',
        'description_above' => 0,
        'private' => 0,
        'wrapper_classes' => '',
        'css_classes' => '',
        'aslist' => 0,
        'empty_option' => '',
        'optrand' => 0,
        'other_option' => NULL,
        'other_text' => 'Other ...',
        'description' => '',
        'custom_keys' => FALSE,
        'options_source' => '',
        'analysis' => TRUE,
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $call_answered_component_id = $joined_component_id;
    $joined_component_id++;

    // Target available?
    $joined_components[$joined_component_id] = array(
      'pid' => $parent,
      'form_key' => 'target_available',
      'name' => 'Is het contact beschikbaar om vragen te beantwoorden?',
      'type' => 'select',
      'value' => '',
      'extra' => array(
        'items' => "answered_completed|Ja\nanswered_call_back|Nee, terugbellen\nanswered_blacklist|Nee, wil niet meer gebeld worden",
        'multiple' => 0,
        'title_display' => 'before',
        'description_above' => 0,
        'private' => 0,
        'wrapper_classes' => '',
        'css_classes' => '',
        'aslist' => 0,
        'empty_option' => '',
        'optrand' => 0,
        'other_option' => NULL,
        'other_text' => 'Other ...',
        'description' => '',
        'custom_keys' => FALSE,
        'options_source' => '',
        'analysis' => TRUE,
      ),
      'required' => 1,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $contact_available_component_id = $joined_component_id;
    $joined_component_id++;

    // Call back.
    $joined_components[$joined_component_id] = array(
      'pid' => $parent,
      'form_key' => 'call_back',
      'name' => 'Terugbellen',
      'type' => 'fieldset',
      'value' => '',
      'extra' => array(
        'title_display' => 0,
        'description_above' => 0,
        'private' => 0,
        'css_classes' => 'call-back-wrapper',
        'collapsed' => 0,
        'collapsible' => 0,
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $call_back_component_id = $joined_component_id;
    $joined_component_id++;

    // Target available today?
    $joined_components[$joined_component_id] = array(
      'pid' => $call_back_component_id,
      'form_key' => 'not_available_today',
      'name' => 'Vandaag niet meer bellen',
      'type' => 'select',
      'value' => '',
      'extra' => array(
        'items' => "not_available_today|Vandaag niet meer bellen",
        'multiple' => 1,
        'title_display' => 'none',
        'description_above' => 0,
        'private' => 0,
        'wrapper_classes' => '',
        'css_classes' => '',
        'aslist' => 0,
        'empty_option' => '',
        'optrand' => 0,
        'other_option' => NULL,
        'other_text' => 'Other ...',
        'description' => '',
        'custom_keys' => FALSE,
        'options_source' => '',
        'analysis' => TRUE,
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $not_available_today_component_id = $joined_component_id;
    $joined_component_id++;

    // Call appointment?
    $joined_components[$joined_component_id] = array(
      'pid' => $call_back_component_id,
      'form_key' => 'call_appointment',
      'name' => 'Maak een belafspraak',
      'type' => 'select',
      'value' => '',
      'extra' => array(
        'items' => "call_appointment|Maak een belafspraak",
        'multiple' => 1,
        'title_display' => 'none',
        'description_above' => 0,
        'private' => 0,
        'wrapper_classes' => '',
        'css_classes' => '',
        'aslist' => 0,
        'empty_option' => '',
        'optrand' => 0,
        'other_option' => NULL,
        'other_text' => 'Other ...',
        'description' => '',
        'custom_keys' => FALSE,
        'options_source' => '',
        'analysis' => TRUE,
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $call_appointment_component_id = $joined_component_id;
    $joined_component_id++;

    // Call back date.
    $joined_components[$joined_component_id] = array(
      'pid' => $call_back_component_id,
      'form_key' => 'call_back_date',
      'name' => 'Belafspraak',
      'type' => 'date',
      'value' => 'today',
      'extra' => array(
        'title_display' => 'before',
        'description_above' => 0,
        'private' => 0,
        'start_date' => '-0 years',
        'end_date' => '+1 years',
        'timezone' => 'user',
        'exclude' => array(),
        'year_textfield' => 0,
        'datepicker' => 1,
        'description' => '',
        'analysis' => FALSE,
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $call_back_date_component_id = $joined_component_id;
    $joined_component_id++;

    // Call back time.
    $joined_components[$joined_component_id] = array(
      'pid' => $call_back_component_id,
      'form_key' => 'call_back_time',
      'name' => 'om',
      'type' => 'time',
      'value' => 'now +1 hour',
      'extra' => array(
        'title_display' => 'before',
        'description_above' => 0,
        'private' => 0,
        'hourformat' => '24-hour',
        'minuteincrements' => '5',
        'timezone' => 'user',
        'start_time' => '',
        'end_time' => '',
        'description' => '',
        'analysis' => FALSE,
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $call_back_time_component_id = $joined_component_id;
    $joined_component_id++;

    // Add campaign fieldset.
    $parent = 0;
    $joined_components[$joined_component_id] = array(
      'pid' => $parent,
      'form_key' => 'campaign',
      'name' => 'Campagnevragen',
      'type' => 'fieldset',
      'value' => '',
      'extra' => array(
        'description_above' => 0,
        'private' => 0,
        'css_classes' => '',
        'title_display' => 0,
        'collapsible' => 0,
        'collapsed' => 0,
        'description' => '',
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $campaign_fieldset_component_id = $joined_component_id;
    $parent = $joined_component_id;
    $joined_component_id++;

    // Adding campaign components.
    foreach ($campaign_components as $campaign_cid => $campaign_component) {
      // Unset unknown id's.
      unset($campaign_component['nid']);
      unset($campaign_component['cid']);
      $campaign_component['pid'] = $parent;
      $campaign_component['weight'] = $joined_component_id;
      $joined_components[$joined_component_id] = $campaign_component;
      $campaign_component_id_map[$campaign_cid] = $joined_component_id;
      $joined_component_id++;
    }

    // Add contact data fieldset.
    $parent = 0;
    $joined_components[$joined_component_id] = array(
      'pid' => $parent,
      'form_key' => 'contact_data',
      'name' => 'Contact gegevens corrigeren',
      'type' => 'fieldset',
      'value' => '',
      'extra' => array(
        'description_above' => 0,
        'private' => 0,
        'css_classes' => '',
        'title_display' => 0,
        'collapsible' => 1,
        'collapsed' => 1,
        'description' => '',
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $parent = $joined_component_id;
    $correct_data_component_id = $joined_component_id;
    $joined_component_id++;

    // Unset contact fields that do not need to be corrected.
    $contact_fields = spbellen_correct_contact_fields();
    // Create correction fields.
    foreach ($contact_fields as $key => $name) {
      $type = $key === 'address' ? 'sp_address_auto' : 'textfield';
      $joined_components[$joined_component_id] = array(
        'pid' => $parent,
        'form_key' => 'spbellen_' . $key,
        'name' => $name,
        'type' => $type,
        'value' => '',
        'extra' => array(
          'title_display' => 'inline',
          'description_above' => 0,
          'private' => 0,
          'wrapper_classes' => '',
          'css_classes' => '',
          'width' => '',
          'maxlength' => '',
          'field_prefix' => '',
          'field_suffix' => '',
          'disabled' => 0,
          'unique' => 0,
          'description' => '',
          'placeholder' => '',
          'attributes' => array(),
          'analysis' => FALSE,
        ),
        'required' => 0,
        'weight' => $joined_component_id,
        'page_num' => 1,
      );
      $joined_component_id++;
    }

    // Message for next caller.
    $parent = 0;
    $joined_components[$joined_component_id] = array(
      'pid' => $parent,
      'form_key' => 'caller_message',
      'name' => 'Informatie voor volgende beller',
      'type' => 'textfield',
      'value' => '',
      'extra' => array(
        'title_display' => 'before',
        'description_above' => 0,
        'private' => 0,
        'wrapper_classes' => '',
        'css_classes' => '',
        'width' => '',
        'max_length' => '',
        'field_prefix' => '',
        'field_suffix' => '',
        'disabled' => 0,
        'unique' => 0,
        'description' => '',
        'placeholder' => '',
        'attributes' => array(),
        'analysis' => FALSE,
      ),
      'required' => 0,
      'weight' => $joined_component_id,
      'page_num' => 1,
    );
    $caller_message_component_id = $joined_component_id;
    $joined_component_id++;

    // Adding intro conditionals.
    $joined_conditional_id = 0;
    $joined_conditionals[$joined_conditional_id] = array(
      'rgid' => $joined_conditional_id,
      'andor' => NULL,
      'rules' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'source_type' => 'component',
          'source' => $call_answered_component_id,
          'operator' => 'equal',
          'value' => 'answered',
        ),
      ),
      'actions' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'target_type' => 'component',
          'target' => $contact_available_component_id,
          'invert' => 0,
          'action' => 'show',
          'argument' => '',
        ),
      ),
    );
    $joined_conditional_id++;

    // Call back conditional.
    $joined_conditionals[$joined_conditional_id] = array(
      'rgid' => $joined_conditional_id,
      'andor' => NULL,
      'rules' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'source_type' => 'component',
          'source' => $contact_available_component_id,
          'operator' => 'equal',
          'value' => 'answered_call_back',
        ),
      ),
      'actions' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'target_type' => 'component',
          'target' => $call_back_component_id,
          'invert' => 0,
          'action' => 'show',
          'argument' => '',
        ),
      ),
    );
    $joined_conditional_id++;

    // Hide call back appointment selection component.
    $joined_conditionals[$joined_conditional_id] = array(
      'rgid' => $joined_conditional_id,
      'andor' => NULL,
      'rules' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'source_type' => 'component',
          'source' => $not_available_today_component_id,
          'operator' => 'equal',
          'value' => 'not_available_today',
        ),
      ),
      'actions' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'aid' => 0,
          'target_type' => 'component',
          'target' => $call_appointment_component_id,
          'invert' => 1,
          'action' => 'show',
          'argument' => '',
        ),
      ),
    );
    $joined_conditional_id++;

    // Hide call back info appointment components.
    $joined_conditionals[$joined_conditional_id] = array(
      'rgid' => $joined_conditional_id,
      'andor' => NULL,
      'rules' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'source_type' => 'component',
          'source' => $call_appointment_component_id,
          'operator' => 'equal',
          'value' => 'call_appointment',
        ),
      ),
      'actions' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'aid' => 0,
          'target_type' => 'component',
          'target' => $call_back_date_component_id,
          'invert' => 0,
          'action' => 'show',
          'argument' => '',
        ),
        1 => array(
          'rgid' => $joined_conditional_id,
          'aid' => 0,
          'target_type' => 'component',
          'target' => $call_back_time_component_id,
          'invert' => 0,
          'action' => 'show',
          'argument' => '',
        ),
      ),
    );
    $joined_conditional_id++;

    // Add intro conditional to show campaign fieldset.
    $joined_conditionals[$joined_conditional_id] = array(
      'rgid' => $joined_conditional_id,
      'andor' => NULL,
      'rules' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'source_type' => 'component',
          'source' => $contact_available_component_id,
          'operator' => 'equal',
          'value' => 'answered_completed',
        ),
      ),
      'actions' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'target_type' => 'component',
          'target' => $campaign_fieldset_component_id,
          'invert' => 0,
          'action' => 'show',
          'argument' => '',
        ),
      ),
    );
    $joined_conditional_id++;

    // Adding correct data conditionals.
    $joined_conditionals[$joined_conditional_id] = array(
      'rgid' => $joined_conditional_id,
      'andor' => NULL,
      'rules' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'source_type' => 'component',
          'source' => $call_answered_component_id,
          'operator' => 'equal',
          'value' => 'answered',
        ),
      ),
      'actions' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'target_type' => 'component',
          'target' => $correct_data_component_id,
          'invert' => 0,
          'action' => 'show',
          'argument' => '',
        ),
      ),
    );
    $joined_conditional_id++;

    // Adding campaign conditionals.
    foreach ($campaign_conditionals as $campaign_cid => $campaign_conditional) {
      // Unset unknown id's.
      unset($campaign_conditional['nid']);
      $campaign_conditional['rgid'] = $joined_conditional_id;
      foreach ($campaign_conditional['rules'] as $rid => &$rule) {
        unset($rule['nid']);
        unset($rule['rid']);
        $rule['rgid'] = $joined_conditional_id;
        $rule['source'] = $campaign_component_id_map[$rule['source']];
      }
      foreach ($campaign_conditional['actions'] as $aid => &$rule) {
        unset($rule['nid']);
        unset($rule['aid']);
        $rule['rgid'] = $joined_conditional_id;
        $rule['target'] = $campaign_component_id_map[$rule['target']];
      }
      $joined_conditionals[$joined_conditional_id] = $campaign_conditional;
      $joined_conditional_id++;
    }

    // Add conditionals to show caller_message.
    $joined_conditionals[$joined_conditional_id] = array(
      'rgid' => $joined_conditional_id,
      'andor' => 'or',
      'rules' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'source_type' => 'component',
          'source' => $call_answered_component_id,
          'operator' => 'equal',
          'value' => 'not_answered',
        ),
        1 => array(
          'rgid' => $joined_conditional_id,
          'source_type' => 'component',
          'source' => $contact_available_component_id,
          'operator' => 'equal',
          'value' => 'answered_call_back',
        ),
      ),
      'actions' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'target_type' => 'component',
          'target' => $caller_message_component_id,
          'invert' => 0,
          'action' => 'show',
          'argument' => '',
        ),
      ),
    );
    $joined_conditional_id++;

    // Create new webform.
    $joined_form = new stdClass();
    $joined_form->type = 'callcampaign';
    node_object_prepare($joined_form);
    if (!empty($campaign->title)) {
      $joined_form->title = $campaign->title;
    }
    else {
      $joined_form->title = 'Campagne: ' . $campaign_webform->title;
    }
    $joined_form->language = $campaign_webform->language;
    $joined_form->uid = $campaign_webform->uid;
    $joined_form->promote = 0;
    $joined_form->comment = 0;
    $joined_form->sticky = 0;
    $joined_form->path['pathauto'] = 0;
    $joined_form->path['alias'] = 'bellen/campagne/' . pathauto_cleanstring($joined_form->title);
    // Attach the webform to the node.
    $joined_form->webform = array(
      'confirmation' => '',
      'confirmation_format' => NULL,
      'redirect_url' => '<none>',
      'status' => '1',
      'block' => '0',
      'teaser' => '0',
      'allow_draft' => '0',
      'auto_save' => '0',
      'submit_notice' => '1',
      'submit_text' => '',
      'submit_limit' => '-1',
      'submit_interval' => '-1',
      'total_submit_limit' => '-1',
      'total_submit_interval' => '-1',
      'progressbar_bar' => 0,
      'preview' => 0,
      'record_exists' => TRUE,
      // Anonymous user can submit this webform.
      'roles' => array(
        0 => '1',
      ),
      'emails' => array(),
      'components' => $joined_components,
      'conditionals' => $joined_conditionals,
      'roles' => array(2),
    );
    node_save($joined_form);
    if (!empty($joined_form->nid)) {
      $campaign->webform_id = $joined_form->nid;
      entity_save('campaign', $campaign);
      $alias = pathauto_cleanstring($joined_form->title);
      $alias_array = array(
        'source' => 'node/' . $joined_form->nid,
        'alias' => 'bellen/campagne/' . $alias,
      );
      path_delete($joined_form->nid);
      path_save($alias_array);
    }
  }
  else {
  }
}

/**
 * Get allowed components.
 */
function spbellen_get_webform_allowed_components($nid, $allowed_types) {
  if (!empty($nid)) {
    $webform = node_load($nid);

    if (!empty($webform->webform['components'])) {
      foreach ($webform->webform['components'] as $key => $component) {
        if (isset($component['type']) && !empty($allowed_types[$component['type']])) {
          $components_filtered[$key] = $component;
        }
      }
      if (!empty($components_filtered)) {
        return $components_filtered;
      }
    }
  }
  return FALSE;
}

/**
 * Parse webform component option items.
 */
function spbellen_parse_webform_component_option_items($items) {
  $options = array();
  $lines = explode("\n", trim($items));
  foreach ($lines as $line) {
    $line = trim($line);
    $matches = array();
    if (preg_match('/^([^|]*)\|(.*)$/', $line, $matches)) {
      $options[$matches[1]] = empty($matches[2]) ? $matches[1] : $matches[2];
    }
  }
  return $options;
}

function spbellen_contact_fields() {
  $contact_fields = array(
    'contact_id' => 'Beltool contact id',
    'phone' => 'Telefoonnummer',
    'display_name' => 'Weergave naam',
    'email' => 'E-mail',
    'street_address' => 'Adres',
    'postal_code' => 'Postcode',
    'city' => 'Plaats',
    'gender' => 'Geslacht',
    'is_lid' => 'Is lid',
    'afdeling' => 'Afdeling',
    'afdeling_id' => 'Afdeling id',
    'crm_contact_id' => 'CRM contact id',
    'notes' => 'Notities',
  );
  return $contact_fields;
}

function spbellen_correct_contact_fields() {
  $contact_fields = array(
    'display_name' => 'Weergave naam',
    'phone' => 'Telefoonnummer',
    'email' => 'E-mail',
    'address' => 'Adresinformatie',
  );
  return $contact_fields;
}

function spbellen_is_spbellen_form($nid) {
  // Check if spbellen form.
  $data = &drupal_static(__FUNCTION__);
  if (!isset($data[$nid])) {
    if ($cache = cache_get('spbellen_forms')) {
      $data = $cache->data;
      if (!isset($data[$nid])) {
        $data[$nid] = spbellen_is_spbellen_form_check($nid);
        cache_set('spbellen_forms', $data, 'cache');
      }
    }
    else {
      $data[$nid] = spbellen_is_spbellen_form_check($nid);
      cache_set('spbellen_forms', $data, 'cache');
    }
  }
  return $data[$nid];
}

function spbellen_is_spbellen_form_check($nid) {
  // Check if spbellen form.
  $query = new EntityFieldQuery();
  $result = $query
    ->entityCondition('entity_type', 'campaign')
    ->propertyCondition('webform_id', $nid)
    ->execute();
  if (isset($result['campaign'])) {
    return TRUE;
  }
  return FALSE;
}

function spbellen_assign_target($campaign) {
  $contact_data = &drupal_static(__FUNCTION__);
  if (!isset($contact_data)) {
    $contact_data = FALSE;
    global $user;
    $campaign_id = $campaign->id;
    $caller = entity_load_single('caller', $user->uid);
    $max_attempts = variable_get('spbellen_max_attempts', '5');

    // Check if is logged in via spoauth.
    if (
      user_is_logged_in()
    ) {
      // Query parts.
      $from_where = spbellen_get_from_where();

      $from_vars = array(
        ':cid' => $campaign_id,
        ':caller_id' => $user->uid,
        ':max_attempts' => $max_attempts,
      );

      // Optionally filter or sort by geo information.
      $order_by_geo = '';
      $order_by_geo_vars = array();

      if (!user_access('spbellen administer campaigns', $user) && !user_access('spbellen administer callers', $user)) {
        switch ($campaign->afdeling_filter) {
          case 0:
            // By preference chapter or regional contacts.
            if (!empty($caller->afdeling_id) && !empty($caller->regio_id)) {
              $order_by_geo = 'afdeling_id = :afdeling_id DESC, regio_id = :regio_id DESC, ';
              $order_by_geo_vars[':afdeling_id'] = $caller->afdeling_id;
              $order_by_geo_vars[':regio_id'] = $caller->regio_id;
            }
            break;

          case 1:
            // All contacts.
            break;

          case 2:
            // Only chapter contacts.
            if (!empty($caller->afdeling_id)) {
              $from_where .= 'cd.afdeling_id = :afdeling_id AND ';
              $from_vars[':afdeling_id'] = $caller->afdeling_id;
            }
            else {
              drupal_set_message('Afdeling beller is onbekend!', 'error');
              drupal_goto('/bellen/error');
              return $contact_data;
            }
            break;
        }
      }

      // Order defaults.
      $order_defaults = 'call_order ASC, attempts ASC, ' . $order_by_geo . ' afdeling_id IS NULL DESC, ta.date ASC, RAND()';

      // Check if claimed target exists.
      $query = "SELECT ta.contact_id" . $from_where . "occupied = 1 AND (attempts = 0 OR (result = 'not_answered' OR result = 'answered_call_back' OR result = 'call_appointment' OR result = 'skipped')) AND ta.caller_id = :caller_id LIMIT 1";
      $result = db_query($query, $from_vars)->fetchAssoc();
      if (!$result) {
        // Get appointment targets.
        $query = "SELECT ta.id, ta.contact_id, occupied " . $from_where . "result = 'call_appointment' AND call_back_priority = 1 AND ac.call_back_time < :current_time AND ac.call_back_time + 43200 > :current_time AND (occupied = 0 OR (occupied = 1 AND ta.date < :occupied_time)) ORDER BY call_back_time ASC, " . $order_defaults . " LIMIT 1";
        $result = db_query($query, array_merge($from_vars, $order_by_geo_vars, array(
          ':current_time' => time(),
          ':occupied_time' => time() - 1800,
        )))->fetchAssoc();
        if ($result) {
          // Claim target.
          db_update('spbellen_targets')
            ->fields(array(
              'occupied' => 1,
              'date' => time(),
              'caller_id' => $user->uid,
            ))
            ->condition('id', $result['id'])
            ->execute();
        }
        else {
          // Get target.
          $query = "SELECT ta.id, ta.contact_id, occupied" . $from_where . "(result = 'not_answered' OR result = 'answered_call_back' OR result = 'call_appointment' OR result = 'skipped' OR result IS NULL) AND ((occupied = 0 AND (ac.call_back_time < :current_time OR ac.call_back_time IS NULL)) OR (occupied = 1 AND ta.date < :occupied_time)) ORDER BY " . $order_defaults . " LIMIT 1";
          $result = db_query($query, array_merge($from_vars, $order_by_geo_vars, array(
            ':occupied_time' => time() - 1800,
            ':current_time' => time(),
          )))->fetchAssoc();
          if ($result) {
            // Claim target.
            db_update('spbellen_targets')
              ->fields(array(
                'occupied' => 1,
                'date' => time(),
                'caller_id' => $user->uid,
              ))
              ->condition('id', $result['id'])
              ->execute();
          }
        }
      }
      else {
      }
      if (!empty($result)) {
        // A target was found!
        $contact_id = $result['contact_id'];
        // Get contact data.
        $result = spbellen_get_contact_data($contact_id);
        if ($result) {
          $contact_data = $result;
        }
        else {
          drupal_set_message('Er kon geen contact informatie worden opgehaald.', 'error');
        }
      }
      else {
        // Find occupied targets.
        $query = "SELECT COUNT(*) number" . $from_where . "(result = 'not_answered' OR result = 'answered_call_back' OR result = 'call_appointment' OR result = 'skipped')";
        $result = db_query($query, $from_vars)->fetchAssoc();
        if ($result) {
          drupal_set_message('Er is geen beschikbaar contact gevonden (er zijn ' . $result['number'] . ' contacten die op een later moment nog gebeld kunnen worden).', 'error');
        }
        else {
          drupal_set_message('Er is geen beschikbaar contact gevonden.', 'error');
        }
      }
    }
    else {
      drupal_set_message('Log in gebruik te kunnen maken van de spbellen.', 'error');
    }
  }
  if (!$contact_data) {
    drupal_goto('/bellen/error');
  }
  return $contact_data;
}

/**
 * Disassign target.
 */
function spbellen_disassign_target($tid, $last_action_id) {
  $result = db_update('spbellen_targets')
    ->fields(array(
      'occupied' => 0,
      'date' => time(),
      'caller_id' => NULL,
      'last_action_id' => $last_action_id,
    ))
    ->expression('attempts', 'attempts + 1')
    ->condition('id', $tid)
    ->execute();
}

/**
 * Check result.
 */
function spbellen_check_result($result, $dataname, $action, $data = NULL, $submission = NULL) {
  $action_name = ($action === 'get') ? 'ophalen' : 'aanmaken';
  $place_name = ($action === 'get') ? 'uit' : 'in';
  if (!isset($result['is_error']) || $result['is_error'] == 1) {
    if (empty($submission)) {
      drupal_set_message('Fout bij het ' . $action_name . ' van ' . $dataname . '.', 'error');
      watchdog('spbellen', '<p>Fout bij het ' . $action_name . ' van ' . $dataname . '.<pre>' . print_r($result, TRUE) . '</pre>Parameters:<pre>' . print_r($data, TRUE) . '</pre>', array(), WATCHDOG_ERROR);
    }
    else {
      drupal_set_message('Fout bij het ' . $action_name . ' van ' . $dataname . ' ' . $place_name . ' CiviCRM (sid = ' . $submission->sid . ').', 'error');
      watchdog('spbellen', '<p>Fout bij het ' . $action_name . ' van ' . $dataname . ' ' . $place_name . ' CiviCRM (sid = ' . $submission->sid . ').</p>Resultaat:<pre>' . print_r($result, TRUE) . '</pre>Parameters:<pre>' . print_r($data, TRUE) . '</pre>', array(), WATCHDOG_ERROR);
    }
    return FALSE;
  }
  return TRUE;
}

function spbellen_campaign_form_access($form_nid) {
  // Give access to campaign administrator and caller admin.
  global $user;
  if (user_access('spbellen administer campaigns', $user) || user_access('spbellen administer callers', $user)) {
    return TRUE;
  }

  // Check session.
  if (
    !empty($_SESSION['spbellen']['local_access'][$form_nid]) ||
    !empty($_SESSION['spbellen']['all_access'])
  ) {
    return TRUE;
  }
  return FALSE;
}

function spbellen_get_campaign_id($webform_id) {
  $query = "SELECT id FROM {spbellen_campaign} WHERE webform_id = :webform_id";
  $result = db_query($query, array(':webform_id' => $webform_id))->fetchAssoc();
  return (!empty($result['id'])) ? $result['id'] : FALSE;
}

function spbellen_get_target_info($campaign_id, $contact_id) {
  // Get target id.
  $query = "SELECT * FROM {spbellen_targets} WHERE contact_id = :contact_id AND campaign_id = :cid";
  $result = db_query($query, array(':contact_id' => $contact_id, ':cid' => $campaign_id))->fetchAssoc();
  return $result;
}

function spbellen_component_key_id_map($webform_node) {
  foreach ($webform_node->webform['components'] as $key => $component) {
    $map[$component['form_key']] = $key;
  }
  return $map;
}

function spbellen_create_action($tid, $caller_id, $result, $submission_id, $call_back_time, $priority, $caller_message = '') {
  $action_id = db_insert('spbellen_action')
    ->fields(array(
      'target_id' => $tid,
      'caller_id' => $caller_id,
      'timestamp' => time(),
      'result' => $result,
      'webform_submission_id' => $submission_id,
      'call_back_time' => $call_back_time,
      'call_back_priority' => $priority,
      'caller_message' => $caller_message,
    ))
    ->execute();
  return $action_id;
}

function spbellen_blacklist($contact_id) {
  db_insert('spbellen_blacklist')
    ->fields(array(
      'contact_id' => $contact_id,
    ))
    ->execute();
  $crm_contact_id = spbellen_get_crm_contact_id($contact_id);
  if (!empty($crm_contact_id)) {
    module_invoke_all('spbellen_crm_block_phone', $crm_contact_id);
  }
}

function spbellen_action_translate() {
  return array(
    'not_answered' => 'Niet opgenomen',
    'wrong_number' => 'Verkeerd nummer',
    'answered_completed' => 'Gesprek afgerond.',
    'answered_call_back' => 'Telefoon opgenomen, terugbellen.',
    'call_appointment' => 'Terugbelafspraak gemaakt.',
    'answered_blacklist' => 'Niet meer bellen.',
    'skipped' => 'Overgeslagen',
  );
}

/**
 * Get contact data.
 */
function spbellen_get_contact_data($contact_id) {
  $contact_data = &drupal_static(__FUNCTION__);
  if (!isset($contact_data[$contact_id])) {
    $fields = array_keys(spbellen_contact_fields());
    foreach ($fields as $key => $field) {
      $fields[$key] = 'cd.' . $field;
    }
    $fields_list = implode(',', $fields);
    // Get contact data.
    $query = "SELECT " . $fields_list . " FROM {spbellen_contact_data} cd WHERE cd.contact_id = :contact_id";
    $result = db_query($query, array(':contact_id' => $contact_id));
    $contact_data[$contact_id] = $result->fetchAssoc();
  }
  return $contact_data[$contact_id];
}

/**
 * Get active campaign.
 */
function spbellen_get_active_campaign() {
  if (arg(0) == 'node' && is_numeric(arg(1))) {
    $nid = arg(1);
    $campaign_id = spbellen_get_campaign_id($nid);
    if ($campaign_id !== FALSE) {
      return $campaign_id;
    }
  }
  return FALSE;
}

/**
 * Get campaign form alias.
 */
function spbellen_get_campaign_form_alias($webform_id) {
  $webform = node_load($webform_id);
  if (!empty($webform)) {
    $webform_alias = drupal_get_path_alias('node/' . $webform_id);
    if (!empty($webform_alias)) {
      $alias = str_replace('/', '', strrchr($webform_alias, '/'));
      return $alias;
    }
  }
}

function spbellen_form_ajax_callback($form, &$form_state) {
  return $form;
}

function spbellen_click2dial_location() {
  $ip_address = ip_address();
  $location_ip = variable_get('spbellen_click2dial_location_ip', FALSE);
  if ($ip_address === $location_ip) {
    return TRUE;
  }
  return FALSE;
}

function spbellen_click2dial_call_numbers_list() {
  $number_list = array();
  $quantity = variable_get('click2dial_call_numbers_quantity', 2);
  for ($nr = 1; $nr <= $quantity; $nr++) {
    $name = variable_get('spbellen_click2dial_callerIDName_' . $nr, '');
    $number = variable_get('spbellen_click2dial_forceclip_' . $nr, '');
    if (!empty($name) && !empty($number)) {
      $number_list[$number] = $name;
    }
  }
  return $number_list;
}

function spbellen_get_crm_contact_id($contact_id) {
  if (!empty($contact_id)) {
    $crm_contact_id = db_select('spbellen_contact_data', 'cd')
      ->fields('cd', array('crm_contact_id'))
      ->condition('cd.contact_id', $contact_id)
      ->execute()
      ->fetchField();
    if (!empty($crm_contact_id)) {
      return $crm_contact_id;
    }
  }
  return FALSE;
}

function spbellen_get_contact_id($crm_contact_id) {
  if (!empty($crm_contact_id)) {
    $contact_id = db_select('spbellen_contact_data', 'cd')
      ->fields('cd', array('contact_id'))
      ->condition('cd.crm_contact_id', $crm_contact_id)
      ->execute()
      ->fetchField();
    if (!empty($contact_id)) {
      return $contact_id;
    }
  }
  return FALSE;
}

function spbellen_get_chapter_and_region($postal_code) {
  if (! empty($postal_code)) {
    // Get afdeling.
    foreach (module_implements('spbellen_get_chapter_and_region') as $module) {
      $function = $module . '_spbellen_get_chapter_and_region';
      $afdeling_data = $function($postal_code);
      if (!empty($afdeling_data)) {
        return $afdeling_data;
      }
    }
  }
}

/**
 * Get callers.
 */
function spbellen_get_callers() {
  $callers = &drupal_static(__FUNCTION__);
  if (!isset($callers)) {
    $callers = array();
    $query = "SELECT caller_id FROM {spbellen_caller}";
    $result = db_query($query);
    if ($result) {
      while ($row = $result->fetchAssoc()) {
        $caller_ids[] = $row['caller_id'];
      }
      $callers = entity_load('caller', $caller_ids);
    }
  }
  return $callers;
}

/**
 * Get callers list.
 */
function spbellen_get_callers_list() {
  $callers_list = &drupal_static(__FUNCTION__);
  if (!isset($callers_list)) {
    $callers_list = array();
    $callers = spbellen_get_callers();
    if (!empty($callers)) {
      foreach ($callers as $caller) {
        $user = user_load($caller->caller_id);
        $callers_list[$caller->caller_id] = $caller->display_name . ' (' . $user->mail . ')';
        asort($callers_list, SORT_STRING);
      }
    }
  }
  return $callers_list;
}

/**
 * Search callers.
 */
function spbellen_get_campaign_callers($campaign_id) {
  $callers = array();
  $query = "SELECT ca.caller_id, ca.display_name, us.mail FROM {spbellen_caller} ca INNER JOIN {users} us ON ca.caller_id = us.uid INNER JOIN spbellen_action ac ON ac.caller_id = ca.caller_id INNER JOIN spbellen_targets ta ON ac.target_id = ta.id WHERE ta.campaign_id = :campaign_id AND ta.state = 1";
  $result = db_query($query, array(':campaign_id' => $campaign_id));
  if ($result) {
    while ($row = $result->fetchAssoc()) {
      $id = check_plain($row['display_name']) . ',  ' . check_plain($row['mail']) . ' (' . $row['caller_id'] . ')';
      $callers[$id] = $id;
    }
  }
  return $callers;
}

/**
 * Search caller campaigns.
 */
function spbellen_caller_campaign_search_autocomplete($caller_id, $search_string) {
  $campaigns = array();
  if (strlen($search_string) > 2) {
    $campaigns = spbellen_caller_get_campaigns($caller_id);
    foreach ($campaigns as $campaign) {
      $id = check_plain($campaign->display_name) . ' (' . $campaign->id . ')';
      $campaign_list[$id] = $id;
    }
  }
  return drupal_json_output($campaign_list);
}

function spbellen_get_all_stats() {
  $stats = &drupal_static(__FUNCTION__);
  if (!isset($stats)) {
    $stats = array();
    // Add campaign stats.
    $campaigns = spbellen_get_campaigns();
    if (!empty($campaigns)) {
      foreach ($campaigns as $campaign_id => $campaign) {
        $stats['campaign_stats'][$campaign_id] = spbellen_get_campaign_stats($campaign_id);
      }
    }
    // Add general stats.
    $stats['general_stats'] = spbellen_get_general_stats();
  }
  return $stats;
}

function spbellen_get_general_stats() {
  $stats = &drupal_static(__FUNCTION__);
  if (!isset($stats)) {
    $stats = array();
    $query = "SELECT COUNT(*) total, SUM(bl.contact_id IS NOT NULL) blacklisted, SUM(bl.contact_id IS NULL AND (result = 'answered_call_back' OR result = 'not_answered')) call_back, SUM(bl.contact_id IS NULL AND result = 'answered_completed') completed, SUM(bl.contact_id IS NULL AND result = 'wrong_number') wrong_number, SUM(bl.contact_id IS NULL AND attempts = 0) not_contacted FROM {spbellen_targets} ta LEFT JOIN {spbellen_action} ac ON ta.last_action_id = ac.id LEFT JOIN {spbellen_blacklist} bl ON ta.contact_id = bl.contact_id WHERE ta.state = 1;";
    $result = db_query($query);
    $record = $result->fetchAssoc();
    foreach ($record as $key => $value) {
      $stats[$key] = empty($value) ? 0 : $value;
      if ($record['total'] != 0) {
        if ($key === 'not_contacted') {
          $stats[$key.'_percentage'] = ceil(100 * $value / $record['total']);
        }
        else {
          $stats[$key.'_percentage'] = floor(100 * $value / $record['total']);
        }
      }
      else {
        $stats[$key.'_percentage'] = 0;
      }
    }
    $stats['title'] = 'Algemene statistieken';
  }
  return $stats;
}

function spbellen_get_campaign_stats($campaign_id, $afdeling_id = NULL) {
  $stats = &drupal_static(__FUNCTION__);
  if (!isset($stats['campaign_id'])) {
    $stats = array();
    $campaign = entity_load_single('campaign', $campaign_id);
    $webform = entity_load_single('node', $campaign->webform_id);
    foreach (array(
      'all' => '',
      'only_active' => ' AND ta.id IN (SELECT ta.id FROM spbellen_targets ta INNER JOIN spbellen_contact_data da ON ta.contact_id = da.contact_id INNER JOIN spbellen_target_import_relation_new tir ON ta.id = tir.target_id INNER JOIN spbellen_imports im ON tir.import_id = im.id INNER JOIN spbellen_groups gr ON im.group_id = gr.id INNER JOIN spbellen_group_campaign_relation gcr ON gr.id = gcr.group_id WHERE gcr.campaign_id = :campaign_id AND gcr.state = 1 AND ta.campaign_id = :campaign_id) AND ta.id NOT IN (SELECT ta.id FROM spbellen_targets ta INNER JOIN spbellen_contact_data da ON ta.contact_id = da.contact_id INNER JOIN spbellen_target_import_relation_new tir ON ta.id = tir.target_id INNER JOIN spbellen_imports im ON tir.import_id = im.id INNER JOIN spbellen_groups gr ON im.group_id = gr.id INNER JOIN spbellen_group_campaign_relation gcr ON gr.id = gcr.group_id WHERE gcr.campaign_id = :campaign_id AND ta.campaign_id = :campaign_id AND gcr.block = 1)',
    ) as $type => $query_part) {
      $query = "SELECT COUNT(*) total, SUM(bl.contact_id IS NOT NULL) blacklisted, SUM(bl.contact_id IS NULL AND (result = 'answered_call_back' OR result = 'not_answered')) call_back, SUM(bl.contact_id IS NULL AND result = 'answered_completed') completed, SUM(bl.contact_id IS NULL AND result = 'wrong_number') wrong_number, SUM(bl.contact_id IS NULL AND attempts = 0) not_contacted FROM {spbellen_targets} ta INNER JOIN spbellen_contact_data da ON ta.contact_id = da.contact_id LEFT JOIN {spbellen_action} ac ON ta.last_action_id = ac.id LEFT JOIN {spbellen_blacklist} bl ON ta.contact_id = bl.contact_id WHERE ta.campaign_id = :campaign_id AND ta.state = 1" . $query_part;
      $vars = array(
        ':campaign_id' => $campaign_id,
      );
      if (!empty($afdeling_id)) {
        $query .= ' AND da.afdeling_id = :afdeling_id';
        $vars[':afdeling_id'] = $afdeling_id;
      }
      $result = db_query($query, $vars);
      $record = $result->fetchAssoc();
      foreach ($record as $key => $value) {
        $stats[$campaign_id][$type][$key] = empty($value) ? 0 : $value;
        if ($record['total'] != 0) {
          $stats[$campaign_id][$type][$key . '_percentage'] = round(100 * $value / $record['total']);
        }
        else {
          $stats[$campaign_id][$type][$key . '_percentage'] = 0;
        }
      }
      $stats[$campaign_id]['title'] = $webform->title;
      $stats[$campaign_id]['webform_id'] = $campaign->webform_id;
    }
  }
  return $stats[$campaign_id];
}

function spbellen_get_caller_stats($caller_id = NULL, $campaign_id = NULL, $call_date = NULL) {
  $stats = array();
  $vars = array();
  $select = "ca.caller_id caller_id, ca.display_name name, ca.phone phone, COUNT(*) total_including_not_answered, SUM(result != 'not_answered') total, SUM(result = 'call_back') call_back, SUM(result = 'not_answered') not_answered, SUM(result = 'skipped') skipped, SUM(result = 'answered_blacklist') blacklisted, SUM(result = 'answered_call_back') answered_call_back, SUM(result = 'not_answered') not_answered, SUM(result = 'answered_completed') answered_completed, SUM(result = 'wrong_number') wrong_number, SUM(result = 'call_appointment') call_appointment";
  $tables = "spbellen_targets ta INNER JOIN spbellen_action ac ON ta.id = ac.target_id INNER JOIN spbellen_caller ca ON ca.caller_id = ac.caller_id";
  $where = "ta.state = 1";
  if ($caller_id === 'all') {
    $conditions = ' GROUP BY ca.caller_id';
  }
  if (!empty($caller_id && $caller_id !== 'all')) {
    $where .= ' AND ac.caller_id = :caller_id';
    $vars[':caller_id'] = $caller_id;
  }
  if (!empty($campaign_id)) {
    $where .= ' AND ta.campaign_id = :campaign_id';
    $vars[':campaign_id'] = $campaign_id;
  }
  if (!empty($call_date)) {
    $where .= ' AND DATE(FROM_UNIXTIME(ac.timestamp)) = :call_date';
    $vars[':call_date'] = $call_date;
  }
  $query = 'SELECT ' . $select . ' FROM ' . $tables . ' WHERE ' . $where . $conditions . ' ORDER BY ca.display_name ASC';
  $result = db_query($query, $vars);
  while ($record = $result->fetchAssoc()) {
    foreach ($record as $key => $value) {
      if (!in_array($key, array('caller_id', 'name', 'phone'))) {
        $data[$key] = empty($value) ? 0 : $value;
        if ($record['total'] != 0) {
          if ($key == 'not_answered') {
            $data[$key . '_percentage'] = round(100 * $value / $record['total_including_not_answered']);
          }
          else {
            $data[$key . '_percentage'] = round(100 * $value / $record['total']);
          }
        }
        else {
          $data[$key . '_percentage'] = 0;
        }
      }
      else {
        $data[$key] = empty($value) ? '' : $value;
      }
      if (empty($caller_id)) {
        $stats = $data;
      }
      else {
        $stats[$record['caller_id']] = $data;
      }
    }
  }
  if (!empty($caller_id) && $caller_id !== 'all') {
    $caller = entity_load_single('caller', $caller_id);
    $stats['caller_display_name'] = $caller->display_name;
  }
  if (!empty($campaign_id)) {
    $campaign = entity_load_single('campaign', $campaign_id);
    $stats['campaign_title'] = $campaign->title;
  }
  if (!empty($call_date)) {
    $stats['call_date'] = $call_date;
  }
  return $stats;
}

/**
 * Get caller campaigns stats.
 */
function spbellen_get_result_stats($caller_id = NULL, $campaign_id, $call_date = NULL) {
  $stats = array();
  $campaign = entity_load_single('campaign', $campaign_id);
  $webform_id = $campaign->webform_id;

  $query = "SELECT * FROM webform_component WHERE nid = :webform_id AND type = 'select' AND form_key NOT IN ('call_answered', 'target_available', 'not_available_today', 'call_appointment') ORDER BY weight";
  $result = db_query($query, array(':webform_id' => $webform_id));
  if ($result) {
    while ($row = $result->fetchAssoc()) {
      $extra = unserialize($row['extra']);
      $items = $extra['items'];
      $options = spbellen_parse_webform_component_option_items($items);
      $result_stats = spbellen_get_caller_result_stats($caller_id, $campaign_id, $call_date, $row['cid'], $options);
      $options['empty'] = 'Niet ingevuld';
      $options['answered_completed'] = 'Vragen beantwoord';
      $options['answered'] = 'Opgenomen';
      $stats['components'][$row['cid']] = array(
        'cid' => $row['cid'],
        'name' => check_plain($row['name']),
        'options' => $options,
        'stats' => $result_stats,
      );
    }
  }
  if (!empty($call_date)) {
    $stats['call_date'] = $call_date;
  }
  return $stats;
}

/**
 * Get caller campaign result stats.
 */
function spbellen_get_caller_result_stats($caller_id = NULL, $campaign_id, $call_date = NULL, $cid, $options) {
  $stats = array();
  // Create query.
  $query = 'SELECT ';
  if ($caller_id == 'all') {
    $query .= 'ca.caller_id, ca.display_name, ';
  }
  foreach ($options as $key => $option) {
    $key_sanitized = preg_replace('/[^a-zA-Z_]/', '', $key);
    $query_parts[] = "SUM(result = 'answered_completed' AND da.data = '" . $key_sanitized . "') AS '" . $key_sanitized . "'";
  }
  $query .= implode(',', $query_parts);
  $query .= ", SUM(result = 'answered_completed' AND da.data NOT IN ('" . implode("','", array_keys($options)) . "')) AS empty";
  $query .= ", SUM(result = 'answered_completed') AS answered_completed, SUM(result != 'not_answered') AS answered FROM spbellen_targets ta JOIN spbellen_campaign cam ON ta.campaign_id = cam.id JOIN spbellen_action ac ON ta.id = ac.target_id LEFT JOIN webform_submitted_data da ON da.sid = ac.webform_submission_id AND da.cid = :cid AND da.nid = cam.webform_id INNER JOIN spbellen_caller ca ON ac.caller_id = ca.caller_id LEFT JOIN spbellen_blacklist bl ON ta.contact_id = bl.contact_id WHERE ta.campaign_id = :campaign_id AND ta.state = 1";
  $vars = array(
    ':campaign_id' => $campaign_id,
    ':cid' => $cid,
  );
  if (!empty($caller_id) && $caller_id != 'all') {
    $query .= " AND ac.caller_id = :caller_id";
    $vars[':caller_id'] = $caller_id;
  }
  if (!empty($call_date)) {
    $query .= ' AND DATE(FROM_UNIXTIME(ac.timestamp)) = :call_date';
    $vars[':call_date'] = $call_date;
  }
  if ($caller_id == 'all') {
    $query .= ' GROUP BY ca.caller_id';
  }

  $query .= ' ORDER BY ca.display_name ASC';

  $results = db_query($query, $vars);
  if ($results) {
    while ($row = $results->fetchAssoc()) {
      foreach ($row as $key => $value) {
        $data[$key] = empty($value) ? 0 : $value;
        if (!in_array($key, array('caller_id', 'display_name'))) {
          if ($row['answered_completed'] != 0) {
            $data[$key . '_percentage_answered_completed'] = round(100 * $value / $row['answered_completed']);
          }
          else {
            $data[$key . '_percentage_answered_completed'] = 0;
          }
          if ($row['answered'] != 0) {
            $data[$key . '_percentage_answered'] = round(100 * $value / $row['answered']);
          }
          else {
            $data[$key . '_percentage_answered'] = 0;
          }
        }
        if ($caller_id == 'all') {
          $stats[$row['caller_id']] = $data;
        }
        else {
          $stats = $data;
        }
      }
    }
  }
  return $stats;
}

/**
 * Get last called date for caller.
 */
function spbellen_get_call_dates($campaign_id, $caller_id = NULL) {
  if (!empty($campaign_id)) {
    $query = "SELECT DISTINCT(DATE(FROM_UNIXTIME(timestamp))) as calldate FROM spbellen_action ac INNER JOIN spbellen_targets ta ON ac.target_id = ta.id INNER JOIN spbellen_caller ca ON ac.caller_id = ca.caller_id WHERE ta.state = 1 AND campaign_id = :campaign_id";
    $vars = array(
      ':campaign_id' => $campaign_id,
    );

    // Filter caller.
    if (!empty($caller_id)) {
      $query .= ' AND ac.caller_id = :caller_id';
      $vars[':caller_id'] = $caller_id;
    }

    $query .= ' ORDER BY timestamp DESC';
    $result = db_query($query, $vars);
    if ($result) {
      $dates = $result->FetchAllKeyed(0, 0);
      return $dates;
    }
  }
}

/**
 * Gets caller target contact data.
 */
function spbellen_get_caller_target_contact_data($caller_id) {
  $contact_data = &drupal_static(__FUNCTION__);
  if (!isset($contact_data)) {
    $contact_data = array();
    if (!empty($caller_id)) {
      $query = "SELECT contact_id FROM {spbellen_targets} WHERE occupied = 1 AND caller_id = :caller_id ORDER BY date DESC";
      $contact_id = db_query($query, array('caller_id' => $caller_id))->fetchField(0);
      if (!empty($contact_id)) {
        $contact_data = spbellen_get_contact_data($contact_id);
      }
    }
    return $contact_data;
  }
}

function spbellen_cleanup_phone($phone) {
  // Cleanup phone.
  if (!empty($phone)) {
    $phone = preg_replace('/\D/', '', $phone);
    $phone = preg_replace('/^(\31|0031)/', '0', $phone);
  }
  return $phone;
}

function spbellen_valid_phone($phone) {
  if (!empty($phone)) {
    if (strlen($phone) > 8) {
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Merge contacts.
 */
function spbellen_merge_contacts($contact_ids) {
  // Find best contact.
  $best_contact_id = spbellen_find_best_contact($contact_ids);
  $best_contact_data = spbellen_get_contact_data($best_contact_id);
  // Merge contact data.
  foreach ($contact_ids as $contact_id) {
    if ($contact_id !== $best_contact_id) {
      $contact_data = spbellen_get_contact_data($contact_id);
      if (empty($contact_data['crm_contact_id'])) {
        drush_print("");
        drush_print('Phone: ' . $best_contact_data['phone']);
        drush_print('Merging contact ' . $contact_id . ' with ' . $best_contact_id);
        $output = drush_format($contact_data);
        drush_print($output);
        $output = drush_format($best_contact_data);
        drush_print($output);

        // Check if contacts share campaign.
        $query = "SELECT t1.campaign_id campaign_id, t1.contact_id contact_id, t1.id contact_target_id, ac1.result contact_result, t2.contact_id best_contact_id, t2.id best_contact_target_id, ac2.result best_contact_result FROM spbellen_targets t1 INNER JOIN spbellen_targets t2 ON t1.contact_id = :contact_id AND t2.contact_id = :best_contact_id AND t1.campaign_id = t2.campaign_id LEFT JOIN spbellen_action ac1 ON t1.last_action_id = ac1.id LEFT JOIN spbellen_action ac2 ON t2.last_action_id = ac2.id";
        $result = db_query($query, array('contact_id' => $contact_id, 'best_contact_id' => $best_contact_id));
        if ($result) {
          if ($result->rowCount()) {
            // Contacts share campaign, remove one of the targets and actions that belong to it.
            while ($row = $result->fetchAssoc()) {
              if (empty($row['campaign_id'])) {
                drush_print('Empty!');
              }
              $output = drush_format($row);
              drush_print($output);
              $campaign_id = $row['campaign_id'];
              $contact_target_id = $row['contact_target_id'];
              $contact_result = $row['contact_result'];
              $best_contact_target_id = $row['best_contact_target_id'];
              $best_contact_result = $row['best_contact_result'];
              // Find best target.
              if (empty($contact_result)) {
                $keep_target_id = $best_contact_target_id;
                $remove_target_id = $contact_target_id;
              }
              elseif (empty($best_contact_result)) {
                $keep_target_id = $contact_target_id;
                $remove_target_id = $best_contact_target_id;
              }
              elseif ($best_contact_result === 'answered_completed') {
                $keep_target_id = $best_contact_target_id;
                $remove_target_id = $contact_target_id;
              }
              elseif ($contact_result === 'answered_completed') {
                $keep_target_id = $contact_target_id;
                $remove_target_id = $best_contact_target_id;
              }
              elseif ($best_contact_result === 'call_appointment') {
                $keep_target_id = $best_contact_target_id;
                $remove_target_id = $contact_target_id;
              }
              elseif ($contact_result === 'call_appointment') {
                $keep_target_id = $contact_target_id;
                $remove_target_id = $best_contact_target_id;
              }
              elseif ($best_contact_result === 'answered_call_back') {
                $keep_target_id = $best_contact_target_id;
                $remove_target_id = $contact_target_id;
              }
              elseif ($contact_result === 'answered_call_back') {
                $keep_target_id = $contact_target_id;
                $remove_target_id = $best_contact_target_id;
              }
              elseif ($best_contact_result === 'answered_blacklist') {
                $keep_target_id = $best_contact_target_id;
                $remove_target_id = $contact_target_id;
              }
              elseif ($contact_result === 'answered_blacklist') {
                $keep_target_id = $contact_target_id;
                $remove_target_id = $best_contact_target_id;
              }
              else {
                $keep_target_id = $best_contact_target_id;
                $remove_target_id = $contact_target_id;
              }
              // Remove target with less interesting result.
              $query = "DELETE ta, ac FROM spbellen_targets ta LEFT JOIN spbellen_action ac ON ac.target_id = ta.id WHERE ta.id = $remove_target_id";
              drush_print($query);
              $query = "DELETE ta, ac FROM spbellen_targets ta LEFT JOIN spbellen_action ac ON ac.target_id = ta.id WHERE ta.id = :remove_target_id";
              db_query($query, array('remove_target_id' => $remove_target_id));
              // Assign best target to best contact.
              $query = "UPDATE spbellen_targets ta SET ta.contact_id = $best_contact_id WHERE ta.id = $keep_target_id";
              drush_print($query);
              $query = "UPDATE spbellen_targets ta SET ta.contact_id = :best_contact_id WHERE ta.id = :keep_target_id";
              db_query($query, array('best_contact_id' => $best_contact_id, 'keep_target_id' => $keep_target_id));
              // Delete inferior contact data.
              $query = "DELETE FROM spbellen_contact_data WHERE contact_id = $contact_id";
              drush_print($query);
              $query = "DELETE FROM spbellen_contact_data WHERE contact_id = :contact_id";
              db_query($query, array('contact_id' => $contact_id));
            }
          }
          else {
            // Contacts do not share campaign, just change contact_id.
            $query = "UPDATE spbellen_targets SET contact_id = $best_contact_id WHERE contact_id = $contact_id";
            drush_print($query);
            $query = "UPDATE spbellen_targets SET contact_id = :best_contact_id WHERE contact_id = :contact_id";
            db_query($query, array('best_contact_id' => $best_contact_id, 'contact_id' => $contact_id));
            $query = "DELETE FROM spbellen_contact_data WHERE contact_id = $contact_id";
            drush_print($query);
            $query = "DELETE FROM spbellen_contact_data WHERE contact_id = :contact_id";
            db_query($query, array('contact_id' => $contact_id));
          }
        }
        drush_print('-------------------------------------');
      }
    }
  }
}

/**
 * Find best contact.
 */
function spbellen_find_best_contact($contact_ids) {
  foreach ($contact_ids as $contact_id) {
    $contact_data = spbellen_get_contact_data($contact_id);
    $scores[$contact_id] = 0;
    if (!empty($contact_data['crm_contact_id'])) {
      $scores[$contact_id] = 3;
    }
    foreach ($contact_data as $value) {
      if (!empty($value)) {
        $scores[$contact_id]++;
      }
    }
  }
  arsort($scores);
  return key($scores);
}

/**
 * Return from where query part to select contact.
 */
function spbellen_get_from_where() {
  $from_where = " FROM {spbellen_targets} ta INNER JOIN spbellen_target_import_relation_new tir ON ta.id = tir.target_id INNER JOIN spbellen_imports im ON tir.import_id = im.id INNER JOIN spbellen_groups gr ON im.group_id = gr.id INNER JOIN spbellen_group_campaign_relation gcr ON gr.id = gcr.group_id INNER JOIN {spbellen_contact_data} cd ON ta.contact_id = cd.contact_id LEFT JOIN {spbellen_action} ac ON ta.last_action_id = ac.id LEFT JOIN {spbellen_blacklist} bl ON ta.contact_id = bl.contact_id WHERE ta.contact_id NOT IN (SELECT DISTINCT(contact_id) FROM spbellen_action ac INNER JOIN spbellen_targets ta ON ac.target_id = ta.id WHERE ac.caller_id = :caller_id AND result = 'skipped') AND gcr.campaign_id = :cid AND ta.state = 1 AND gcr.state = 1 AND bl.contact_id IS NULL AND attempts <= :max_attempts AND ta.id NOT IN (SELECT tir.target_id FROM spbellen_target_import_relation_new tir INNER JOIN spbellen_imports im ON tir.import_id = im.id INNER JOIN spbellen_groups gr ON im.group_id = gr.id INNER JOIN spbellen_group_campaign_relation gcr ON gr.id = gcr.group_id WHERE gcr.campaign_id = :cid AND gcr.block = 1) AND ta.campaign_id = :cid AND ";
  return $from_where;
}

/**
 * Get information for groups in campagn.
 */
function spbellen_get_campaign_groups_info($campaign_id) {
  $rows = &drupal_static(__FUNCTION__);
  if (!isset($rows)) {
    $rows = array();
    if (!empty($campaign_id)) {
      $query = "SELECT gro.id id, title, date, state, call_order, block, COUNT(DISTINCT(target_id)) quantity FROM {spbellen_groups} gro INNER JOIN {spbellen_group_campaign_relation} gcr ON gro.id = gcr.group_id INNER JOIN {spbellen_imports} imp ON imp.group_id = gro.id INNER JOIN {spbellen_target_import_relation_new} tir ON tir.import_id = imp.id WHERE gcr.campaign_id = :campaign_id GROUP BY gro.id ORDER BY date DESC";
      $result = db_query($query, array(':campaign_id' => $campaign_id));
      $rows = $result->FetchAllAssoc('id', PDO::FETCH_ASSOC);
    }
  }
  else {
  }
  return $rows;
}
