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

function spbellen_get_civi_groups($sync = TRUE) {
  if ($sync) {
    $groups = &drupal_static(__FUNCTION__);
    if (!isset($groups)) {

      if ($cache = cache_get('spbellen_civi_groups')) {
        $groups = $cache->data;
      }
      else {
        $groups = array();
        set_time_limit(0);
        $spcivi = \SPCivi::getInstance();
        $params = array(
          'sequential'            => 1,
          'option.limit'          => 9999,
        );
        $groups_results = $spcivi->api('Group', 'get', $params);
        if (spbellen_check_result($groups_results, 'groepen', 'get', $params)) {
          if (!empty($groups_results['values'])) {
            foreach ($groups_results['values'] as $value) {
              if (
                empty($value['group_type']) || 
                in_array('2', $value['group_type'])
              ) {
              $groups[$value['id']] = $value['title'];
              }
            }
            asort($groups);
            variable_set('spbellen_civi_groups', $groups);
          }
          else {
            drupal_set_message('Error syncing civicrm groups, using saved data.', 'warning');
          }
        }
      }
      $groups = variable_get('spbellen_civi_groups', array());
      cache_set('spbellen_civi_groups', $groups, 'cache', time() + 3600);
    }
    return $groups;
  }
  $groups = variable_get('spbellen_civi_groups', array());
  return $groups;
}

function spbellen_group_autocomplete($string) {
  $string = strtolower($string);
  $matches = array();
  if (strlen($string) > 2) {
    $matches = array();
    $groups = spbellen_get_civi_groups();

    foreach ($groups as $key => $group) {
      if (strpos(strtolower($group), $string) !== FALSE) {
        $matches[$group . ' (' . $key . ')'] = $group;
      }
      if (count($matches) > 10) break;
    }
  }

  //Return the result to the form in json
  drupal_json_output($matches);
}

function spbellen_parse_group_form_select_value($group_raw) {
  $group = '';
  preg_match('@^.+\(([0-9]+)\)$@', $group_raw, $matches);
  if (!empty($matches[1])) {
    $group = $matches[1];
  }
  return $group;
}

function spbellen_get_stored_campaign_groups($campaign_id, $type) {
  $campaign_groups = array();
  $query = new EntityFieldQuery();

  $result = $query
    ->entityCondition('entity_type', 'campaign_groups')
    ->propertyCondition('campaign_id', $campaign_id) 
    ->propertyCondition('group_type', $type) 
    ->execute();

  if (!empty($result['campaign_groups'])) {
    $cgids = array_keys($result['campaign_groups']);
    $campaign_groups = entity_load('campaign_groups', $cgids);
  }
  return $campaign_groups;
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

    // Add contact components.
    $contact_fields = spbellen_contact_fields();
    foreach ($contact_fields as $key => $name) {
      $joined_components[$joined_component_id] = array(
        'pid' => 0,
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

    $parent = 0;

    // Add intro components.
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
      'name' => 'Heb je met het contact gesproken?',
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

    // Add campaign fieldset.
    $joined_components[$joined_component_id] = array(
      'pid' => 0,
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
    $joined_components[$joined_component_id] = array(
      'pid' => 0,
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
    $contact_fields =  spbellen_no_correct_contact_fields();
    // Create correction fields.
    foreach ($contact_fields as $key => $name) {
      $joined_components[$joined_component_id] = array(
        'pid' => $parent,
        'form_key' => 'spbellen_'.$key,
        'name' => $name,
        'type' => 'textfield',
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

    // Create new webform.
    $joined_form = new stdClass();
    $joined_form->type = 'webform';
    node_object_prepare($joined_form);
    $joined_form->title = 'Campagne: '.$campaign_webform->title;
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

function spbellen_get_targets($campaign) {
  // Get campaign target groups.
  $query = new EntityFieldQuery();
  $result = $query
    ->entityCondition('entity_type', 'campaign_groups')
    ->propertyCondition('campaign_id', $campaign->id)
    ->propertyCondition('group_type', 'target_group')
    ->execute();
  if (isset($result['campaign_groups'])) {
    $tids_civi = array();
    $tids_db = array();
    $campaign_groups_ids = array_keys($result['campaign_groups']);
    $campaign_groups = entity_load('campaign_groups', $campaign_groups_ids);
    // Get targets from CiviCRM.
    $tids_civi = array();
    set_time_limit(0);
    $spcivi = \SPCivi::getInstance();
    foreach ($campaign_groups as $campaign_group) {
      $params = array(
        'option.limit' => 9999,
        'filter.group_id' => array(0 => $campaign_group->civi_group_id),
        'is_deleted' => 0,
        'do_not_phone' => 0,
        'return' => 'contact_id, phone',
        'status' => 'Added',
      );
      $results = $spcivi->api('Contact', 'get', $params);
      if (spbellen_check_result($results, 'contacten', 'get', $params)) {
        // New targets.
        if (!empty($results['values'])) {
          foreach ($results['values'] as $result) {
            if (!empty($result['phone'])) {
              $tids_civi[$result['contact_id']] = $result['contact_id'];
            }
          }
        }
      }
      else {
        return FALSE;
      }
      // Get targets from database.
      $tids_db = array();
      $query = new EntityFieldQuery();

      $results = $query
        ->entityCondition('entity_type', 'target')
        ->propertyCondition('campaign_id', $campaign->id)
        ->execute();

      if (!empty($results['target'])) {
        $tids = array_keys($results['target']);
        $targets = entity_load('target', $tids);
        foreach ($targets as $target) {
          $tids_db[$target->civi_contact_id] = $target->civi_contact_id;
        }
      }
    }
    // Get new and deleted targets.
    $tids_new = array_diff($tids_civi, $tids_db);
    $tids_del = array_diff($tids_db, $tids_civi);

    // Store new ids.
    foreach ($tids_new as $tid) {
      $result = db_insert('spbellen_targets')
        ->fields(array(
          'civi_contact_id' => $tid,
          'campaign_id' => $campaign->id,
          'occupied' => 0,
        ))
        ->execute();
    }

    // Delete removed targets in database.
    foreach ($tids_del as $tid) {
      db_delete('spbellen_targets')
        ->condition('campaign_id', $campaign->id)
        ->condition('civi_contact_id', $tid)
        ->execute();
    }
  }
}

function spbellen_contact_fields() {
  $contact_fields = array(
    'phone' => 'Telefoonnummer',
    'display_name' => 'Weergave naam',
    'email' => 'E-mail',
    'street_address'=> 'Adres',
    'postal_code' => 'Postcode',
    'city' => 'Plaats',
    'contact_id' => 'Contact id',
    'gender' => 'Geslacht',
    'is_lid' => 'Is lid',
  );
  return $contact_fields;
}

function spbellen_no_correct_contact_fields() {
  $contact_fields = array(
    'phone' => 'Telefoonnummer',
    'display_name' => 'Weergave naam',
    'email' => 'E-mail',
    'street_address'=> 'Adres',
    'postal_code' => 'Postcode',
    'city' => 'Plaats',
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

function spbellen_assign_target($campaign_id) {
  global $user;
  $caller = user_load($user->uid);
  // Check if is logged in via spoauth.
  if (
    user_is_logged_in() &&
    !empty($caller->field_contact_id)
  ) {
    // Check if existing target exists.
    $caller_id = $caller->field_contact_id['und'][0]['value'];
    $query = "SELECT ta.civi_contact_id FROM {spbellen_targets} ta LEFT JOIN {spbellen_action} ac ON ta.last_action_id = ac.id LEFT JOIN {spbellen_blacklist} bl ON ta.civi_contact_id = bl.civi_contact_id WHERE bl.civi_contact_id IS NULL AND occupied = 1 AND (attempts = 0 OR (result = 'not_answered' OR result = 'answered_call_back')) AND attempts < 3 AND campaign_id = :cid  AND ta.caller_id = :caller_id LIMIT 1";
    $result = db_query($query, array(
      ':cid' => $campaign_id,
      ':caller_id' => $caller_id,
    ))->fetchAssoc();
    if (!$result) {
      // Get new target.
      $query = "SELECT ta.id, ta.civi_contact_id, occupied FROM {spbellen_targets} ta LEFT JOIN {spbellen_action} ac ON ta.last_action_id = ac.id LEFT JOIN {spbellen_blacklist} bl ON ta.civi_contact_id = bl.civi_contact_id WHERE (attempts = 0 OR (result = 'not_answered' OR result = 'answered_call_back')) AND attempts < 3 AND bl.civi_contact_id IS NULL AND campaign_id = :cid AND ((occupied = 0 AND (attempts = 0 OR (attempts > 0 AND ac.timestamp < :call_again_time))) OR (occupied = 1 AND ta.date < :occupied_time)) ORDER BY attempts ASC, date ASC LIMIT 1";
      $result = db_query($query, array(
        ':cid' => $campaign_id,
        ':occupied_time' => time() - 1800,
        ':call_again_time' => time() - 2700,
      ))->fetchAssoc();
      if ($result) {
        // Claim target.
        $civi_result = db_update('spbellen_targets')
          ->fields(array(
            'occupied' => 1,
            'date' => time(),
            'caller_id' => $caller_id,
          ))
          ->condition('id', $result['id'])
          ->execute();
      }
    }
    if (!empty($result)) {
      $civi_contact_id = $result['civi_contact_id'];
      // Get civicrm data.
      set_time_limit(0);
      $result = spbellen_get_contact_data($civi_contact_id);
      if ($result) {
        return $result;
      }
      else {
        drupal_set_message('Er kon geen contact informatie worden opgehaald.', 'error');
      }
    }
    else {
      // Find occupied targets.
      $query = "SELECT COUNT(*) number FROM {spbellen_targets} ta LEFT JOIN {spbellen_action} ac ON ta.last_action_id = ac.id LEFT JOIN {spbellen_blacklist} bl ON ta.civi_contact_id = bl.civi_contact_id WHERE (result = 'not_answered' OR result = 'answered_call_back') AND attempts < 3 AND bl.civi_contact_id IS NULL AND campaign_id = :cid";
      $result = db_query($query, array(
        ':cid' => $campaign_id,
      ))->fetchAssoc();
      if ($result) {
        drupal_set_message('Er is geen beschikbaar contact gevonden (er zijn '.$result['number'].' contacten die op een later moment nog gebeld kunnen worden).', 'error');
      }
      else {
        drupal_set_message('Er is geen beschikbaar contact gevonden.', 'error');
      }
    }
  }
  else {
    drupal_set_message('Log in via SPNET om gebruik te kunnen maken van de spbellen.', 'error');
  }
  drupal_goto('/bellen/error');
  return FALSE;
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
  // Check session for call groups.
  if (!empty($_SESSION['spbellen']['call_groups'][$form_nid])) {
    return TRUE;
  }
  return FALSE;
}

function spbellen_get_campaign_id($webform_id) {
  $query = "SELECT id FROM {spbellen_campaign} WHERE webform_id = :webform_id";
  $result = db_query($query, array(':webform_id' => $webform_id))->fetchAssoc();
  return (!empty($result['id'])) ? $result['id'] : FALSE;
}

function spbellen_component_key_id_map($webform_node) {
  foreach ($webform_node->webform['components'] as $key => $component) {
    $map[$component['form_key']] = $key;
  }
  return $map;
}

function spbellen_create_action($tid, $caller_id, $result, $submission_id) {
  $action_id = db_insert('spbellen_action')
    ->fields(array(
      'target_id' => $tid,
      'caller_id' => $caller_id,
      'timestamp' => time(),
      'result' => $result,
      'webform_submission_id' => $submission_id,
    ))
    ->execute();
  return $action_id;
}

function spbellen_blacklist($target_contact_id) {
  db_insert('spbellen_blacklist')
    ->fields(array(
      'civi_contact_id' => $target_contact_id,
    ))
    ->execute();
}

function spbellen_action_translate() {
  return array(
    'not_answered' => 'Niet opgenomen',
    'wrong_number' => 'Verkeerd nummer',
    'answered_completed' => 'Gesprek afgerond.',
    'answered_call_back' => 'Telefoon opgenomen, terugbellen.',
    'answered_blacklist' => 'Niet meer bellen.',
  );
}

function spbellen_get_contact_data($contact_id) {
  $contact_data = &drupal_static(__FUNCTION__);
  if (!isset($contact_data)) {
    // Get civicrm data.
    set_time_limit(0);
    $spcivi = \SPCivi::getInstance();
    $params = array(
      'contact_id' => $contact_id,
    );
    $result = $spcivi->api('Contact', 'get', $params);
    if (spbellen_check_result($result, 'een contact', 'get', $params)) {
      if (!empty($result['values'])) {
        $contact_data = $result['values'][0];

        // Get membership data.
        $params = array(
          'contact_id' => $contact_id,
          'membership_type_id' => array('IN' => array("1", "2", "3")),
          'status_id' => 2,
        );
        $result = $spcivi->api('Membership', 'get', $params);
        if (spbellen_check_result($result, 'lidmaatschapsinformatie', 'get', $params)) {
          if (!empty($result['values'])) {
            $contact_data['is_lid'] = 1;
          }
          else {
            $contact_data['is_lid'] = 0;
          }
        }
      }
    }
    else {
      return FALSE;
    }
  }
  return $contact_data;
}
