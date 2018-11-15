<?php

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

function spbellen_get_webforms_list() {
  $webforms_list = array();
  $webforms = spbellen_get_webforms();
  if (!empty($webforms)) {
    foreach ($webforms as $webform) {
      $webforms_list[$webform->nid] = $webform->title;
    }
  }
  return $webforms_list;
}

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
  }
  return $campaigns;
}

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
            $campaigns_list[$campaign->id] = $webform->title;
          }
        }
      }
    }
  }
  return $campaigns_list;
}

function spbellen_contact_search_autocomplete($user_id, $webform_id, $string) {
  $matches = array();
  if (!empty($string)) {
    if (spbellen_campaign_form_access($webform_id)) {
      $campaign_id = spbellen_get_campaign_id($webform_id);
      if (!empty($campaign_id)) {
        $from_where = " FROM {spbellen_targets} ta INNER JOIN {spbellen_contact_data} cd ON ta.contact_id = cd.contact_id LEFT JOIN {spbellen_action} ac ON ta.last_action_id = ac.id LEFT JOIN {spbellen_blacklist} bl ON ta.contact_id = bl.contact_id WHERE bl.contact_id IS NULL AND ta.id NOT IN (SELECT tir.target_id FROM spbellen_target_import_relation tir INNER JOIN spbellen_import_history im ON tir.import_id = im.id WHERE im.state = 0) AND ";
        if (spbellen_click2dial_location()) {
          // Same query now, no diff, kept code for future info, change.
          $query = "SELECT ta.id, cd.phone, cd.display_name" . $from_where . "(attempts = 0 OR (occupied = 0 AND (result = 'not_answered' OR result = 'answered_call_back')) OR (occupied = 1 AND ta.caller_id = :caller_id)) AND campaign_id = :campaign_id AND (phone LIKE :phonenumber OR display_name LIKE :display_name) LIMIT 10";
        }
        else {
          $query = "SELECT ta.id, cd.phone, cd.display_name" . $from_where . "(attempts = 0 OR (occupied = 0 AND (result = 'not_answered' OR result = 'answered_call_back')) OR (occupied = 1 AND ta.caller_id = :caller_id)) AND campaign_id = :campaign_id AND (phone LIKE :phonenumber OR display_name LIKE :display_name) LIMIT 10";
        }
        $substitutions = array(
          ':campaign_id' => $campaign_id,
          ':phonenumber' => "$string%",
          ':display_name' => "%$string%",
          ':caller_id' => $user_id,
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

  //Return the result to the form in json
  drupal_json_output($matches);
}

function spbellen_parse_id_from_select_value($id_raw) {
  $id = '';
  preg_match('@^.+\(([0-9]+)\)$@', $id_raw, $matches);
  if (!empty($matches[1])) {
    $id = $matches[1];
  }
  return $id;
}

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
    $joined_component_id ++;

    // Add contact components.
    $contact_fields = spbellen_contact_fields();
    foreach ($contact_fields as $key => $name) {
      $joined_components[$joined_component_id] = array(
        'pid' => $parent,
        'form_key' => 'spbellen_hidden_'.$key,
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
      $joined_component_id ++;
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
    $joined_component_id ++;

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
    $joined_component_id ++;

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
    $joined_component_id ++;

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
    $joined_component_id ++;

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
    $joined_component_id ++;

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
    $joined_component_id ++;

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
    $joined_component_id ++;

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
    $joined_component_id ++;

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
    $joined_component_id ++;

    // Adding campaign components');
    foreach ($campaign_components as $campaign_cid => $campaign_component) {
      // Unset unknown id's.
      unset($campaign_component['nid']);
      unset($campaign_component['cid']);
      $campaign_component['pid'] = $parent;
      $campaign_component['weight'] = $joined_component_id;
      $joined_components[$joined_component_id] = $campaign_component;
      $campaign_component_id_map[$campaign_cid] = $joined_component_id;
      $joined_component_id ++;
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
    $joined_component_id ++;

    // Unset contact fields that do not need to be corrected.
    $contact_fields =  spbellen_correct_contact_fields();
    // Create correction fields.
    foreach ($contact_fields as $key => $name) {
      $type = $key === 'address' ? 'sp_address_auto' : 'textfield';
      $joined_components[$joined_component_id] = array(
        'pid' => $parent,
        'form_key' => 'spbellen_'.$key,
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
      $joined_component_id ++;
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
    $joined_component_id ++;

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
    $joined_conditional_id ++;

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
    $joined_conditional_id ++;

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
    $joined_conditional_id ++;

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
    $joined_conditional_id ++;

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
    $joined_conditional_id ++;

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
    $joined_conditional_id ++;

    // Adding campaign conditionals');
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
      $joined_conditional_id ++;
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
    $joined_conditional_id ++;

    // Create new webform.
    $joined_form = new stdClass();
    $joined_form->type = 'webform';
    node_object_prepare($joined_form);
    if (!empty($campaign->title)) {
      $joined_form->title = $campaign->title;
    }
    else {
      $joined_form->title = 'Campagne: '.$campaign_webform->title;
    }
    $joined_form->language = $campaign_webform->language;
    $joined_form->uid = $campaign_webform->uid;
    $joined_form->promote = 0;
    $joined_form->comment = 0;
    $joined_form->sticky = 0;
    $joined_form->path['pathauto'] = 0;
    $joined_form->path['alias'] = 'bellen/campagne/'.pathauto_cleanstring($joined_form->title);
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
      'submit_limit' => '-1', // User can submit more than once.
      'submit_interval' => '-1',
      'total_submit_limit' => '-1',
      'total_submit_interval' => '-1',
      'progressbar_bar' => 0,
      'preview' => 0,
      'record_exists' => TRUE,
      'roles' => array(
        0 => '1', // Anonymous user can submit this webform.
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
      //node_delete($campaign_webform->nid);
      $alias = pathauto_cleanstring($joined_form->title);
      $alias_array = array(
        'source' => 'node/'.$joined_form->nid,
        'alias' => 'bellen/campagne/'.$alias,
      );
      path_delete($joined_form->nid);
      path_save($alias_array);
    }
  }
  else {
  }
}

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

function spbellen_parse_webform_select_options($webform, $selected_component) {
  $options = array();
  if (!empty($webform->webform['components'][$selected_component]['extra']['items'])) {
    $items = $webform->webform['components'][$selected_component]['extra']['items'];
    $lines = explode("\n", trim($items));
    foreach ($lines as $line) {
      $line = trim($line);
      $matches = array();
      if (preg_match('/^([^|]*)\|(.*)$/', $line, $matches)) {
        $options[$matches[1]] = empty($matches[2]) ? $matches[1] : $matches[2];
      }
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

    // Check if is logged in via spoauth.
    if (
      user_is_logged_in()
    ) {
      // Query parts.
      $from_where = " FROM {spbellen_targets} ta INNER JOIN spbellen_target_import_relation tir ON ta.id = tir.target_id INNER JOIN spbellen_import_history im ON tir.import_id = im.id INNER JOIN {spbellen_contact_data} cd ON ta.contact_id = cd.contact_id LEFT JOIN {spbellen_action} ac ON ta.last_action_id = ac.id LEFT JOIN {spbellen_blacklist} bl ON ta.contact_id = bl.contact_id WHERE im.campaign_id = :cid AND ta.state = 1 AND im.state = 1 AND bl.contact_id IS NULL AND attempts < 5 AND ta.id NOT IN (SELECT tir.target_id FROM spbellen_target_import_relation tir INNER JOIN spbellen_import_history im ON tir.import_id = im.id WHERE im.campaign_id = :cid AND im.block = 1) AND ";

      $from_vars = array(
        ':cid' => $campaign_id,
        ':caller_id' => $user->uid,
      );

      // Optionally add afdeling filter.
      if (!empty($campaign->afdeling_filter)) {
        if (!empty($caller->afdeling_id)) {
          $from_where .= 'cd.afdeling_id = :afdeling_id AND ';
          $from_vars[':afdeling_id'] = $caller->afdeling_id;
        }
        else {
          drupal_set_message('Afdeling beller is onbekend!', 'error');
          drupal_goto('/bellen/error');
          return $contact_data;
        }
      }

      // Check if claimed target exists.
      $query = "SELECT ta.contact_id" . $from_where . "occupied = 1 AND (attempts = 0 OR (result = 'not_answered' OR result = 'answered_call_back' OR result = 'call_appointment')) AND ta.caller_id = :caller_id LIMIT 1";
      $result = db_query($query, $from_vars)->fetchAssoc();
      if (!$result) {
        // Get appointment targets.
        $query = "SELECT ta.id, ta.contact_id, occupied " . $from_where . "result = 'call_appointment' AND call_back_priority = 1 AND ac.call_back_time < :current_time AND ac.call_back_time + 43200 > :current_time AND (occupied = 0 OR (occupied = 1 AND ta.date < :occupied_time)) ORDER BY call_back_time ASC, call_order ASC, attempts ASC, ta.date ASC LIMIT 1";
        $result = db_query($query, array_merge($from_vars, array(
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
          $query = "SELECT ta.id, ta.contact_id, occupied" . $from_where . "(result = 'not_answered' OR result = 'answered_call_back' OR result = 'call_appointment' OR result IS NULL) AND ((occupied = 0 AND (ac.call_back_time < :current_time OR ac.call_back_time IS NULL)) OR (occupied = 1 AND ta.date < :occupied_time)) ORDER BY call_order ASC, attempts ASC, ta.date ASC, RAND() LIMIT 1";
          $result = db_query($query, array_merge($from_vars, array(
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
        $query = "SELECT COUNT(*) number" . $from_where . "(result = 'not_answered' OR result = 'answered_call_back' OR result = 'call_appointment')";
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

function spbellen_get_contact_data($contact_id) {
  $contact_data = &drupal_static(__FUNCTION__);
  if (!isset($contact_data)) {
    $contact_data = array();
    $fields = array_keys(spbellen_contact_fields());
    foreach ($fields as $key => $field) {
      $fields[$key] = 'cd.' . $field;
    }
    $fields_list = implode(',', $fields);
    // Get contact data.
    $query = "SELECT " . $fields_list . " FROM {spbellen_targets} ta INNER JOIN {spbellen_contact_data} cd ON ta.contact_id = cd.contact_id WHERE ta.contact_id = :contact_id";
    $result = db_query($query, array(':contact_id' => $contact_id));
    $contact_data = $result->fetchAssoc();
  }
  return $contact_data;
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
      'only_active' => ' AND ta.id IN (SELECT tir.target_id FROM spbellen_target_import_relation tir INNER JOIN spbellen_import_history im ON tir.import_id = im.id WHERE im.campaign_id = :campaign_id AND im.state = 1) AND ta.id NOT IN (SELECT tir.target_id FROM spbellen_target_import_relation tir INNER JOIN spbellen_import_history im ON tir.import_id = im.id WHERE im.campaign_id = :campaign_id AND im.block = 1)',
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
  for ($nr = 1; $nr <= $quantity; $nr ++) {
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

function spbellen_get_branch($postal_code) {
  // Get afdeling.
  foreach (module_implements('spbellen_get_branch') as $module) {
    $function = $module . '_spbellen_get_branch';
    $afdeling_data = $function($postal_code);
    if (!empty($afdeling_data)) {
      return $afdeling_data;
    }
  }
}

/**
 * Import contact in campaign.
 *
 * @param int $campaign_id
 *   Campaign id.
 * @param int $import_id
 *   Import id.
 * @param array $contact_data
 *   See spbellen_contact_fields() for keys.
 */
function spbellen_import_contact($campaign_id, $import_id, array $contact_data) {
  $contact_id = FALSE;
  $fields = array_keys(spbellen_contact_fields());
  foreach ($fields as $field) {
    if (!empty($contact_data[$field])) {
      $values[$field] = $contact_data[$field];
    }
  }

  // Cleanup phone.
  if (!empty($values['phone'])) {
    $values['phone'] = preg_replace('/\D/', '', $values['phone']);
    $values['phone'] = preg_replace('/^(\31|0031)/', '0', $values['phone']);
  }

  // Get contact id.
  if (!empty($values['contact_id'])) {
    $contact_id = $values['contact_id'];
  }
  elseif (!empty($values['crm_contact_id'])) {
    // Get contact_id from crm_contact_id.
    $result = db_query("SELECT contact_id FROM {spbellen_contact_data} WHERE crm_contact_id = :crm_contact_id", array(':crm_contact_id' => $values['crm_contact_id']))->fetchField();
    if (!empty($result)) {
      $contact_id = $result;
    }
  }
  elseif (!empty($values['phone']) && strlen($phone) > 8) {
    // Get contact_id from phone.
    $result = db_query("SELECT contact_id FROM {spbellen_contact_data} WHERE phone LIKE :phone", array(':phone' => '%' . db_like($values['phone']) . '%'))->fetchField();
    if (!empty($result)) {
      $contact_id = $result;
    }
  }
  // Store contact data.
  if (empty($contact_id)) {
    $contact_id = db_insert('spbellen_contact_data')
      ->fields($values)
      ->execute();
  }
  else {
    db_update('spbellen_contact_data')
      ->fields($values)
      ->condition('contact_id', $contact_id)
      ->execute();
  }
  // Add target.
  $target_id = db_query("SELECT id FROM {spbellen_targets} WHERE contact_id = :contact_id AND campaign_id = :campaign_id", array(':contact_id' => $contact_id, ':campaign_id' => $campaign_id))->fetchField();
  if (empty($target_id)) {
    $target_id = db_insert('spbellen_targets')
      ->fields(array(
        'campaign_id' => $campaign_id,
        'state' => 1,
        'occupied' => 0,
        'contact_id' => $contact_id,
      ))
      ->execute();
  }
  // Add target import relation.
  if (!empty($target_id)) {
    db_merge('spbellen_target_import_relation')
      ->key(array(
        'target_id' => $target_id,
        'import_id' => $import_id,
      ))
      ->execute();
  }
  return $contact_id;
}

/**
 * Store import history.
 */
function spbellen_store_import_history($campaign_id, $quantity, $title, $state = 1) {
  if (!empty($campaign_id) && !empty($title)) {
    $fields = array(
      'campaign_id' => $campaign_id,
      'date' => time(),
      'quantity' => $quantity,
      'title' => $title,
      'state' => $state,
    );
    $import_history = entity_create('import_history', $fields);
    entity_save('import_history', $import_history);
    return $import_history->id;
  }
}

function spbellen_get_import_history($campaign_id) {
  $rows = &drupal_static(__FUNCTION__);
  if (!isset($rows)) {
    $rows = array();
    if (!empty($campaign_id)) {
      $query = "SELECT * FROM {spbellen_import_history} WHERE campaign_id = :campaign_id ORDER BY call_order";
      $result = db_query($query, array(':campaign_id' => $campaign_id));
      $rows = $result->FetchAllAssoc('id');
    }
  }
  else {
  }
  return $rows;
}
