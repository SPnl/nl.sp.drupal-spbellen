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
  // Filter already set forms.
  // To do.
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
        if (_spcf_check_result($groups_results, 'groepen', 'get', $params)) {
          if (!empty($groups_results['values'])) {
            foreach ($groups_results['values'] as $value) {
              if (
                !empty($value['group_type']) &&
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

function spbellen_add_intro_form($campaign) {
  $error = FALSE;
  $campaign_webform = node_load($campaign->webform_id);
  if (!empty($campaign_webform->webform)) {
    $form = $campaign_webform->webform; 
    $campaign_components = !empty($form['components']) ? $form['components'] : array();
    $campaign_conditionals = !empty($form['conditionals']) ? $form['conditionals'] : array();
  }
  else {
    $error = TRUE;
    dpm('Could not load campaign webform.');
  }

  $intro_webform = node_load(variable_get('spbellen_intro_webform', ''));
  if (!empty($intro_webform->webform)) {
    $form = $intro_webform->webform;
    $intro_components = !empty($form['components']) ? $form['components'] : array();
    $intro_conditionals = !empty($form['conditionals']) ? $form['conditionals'] : array();
  }
  else {
    $error = TRUE;
    dpm('Could not load intro webform.');
  }

  if (!$error) {
    // Create new webform that joins the intro and campaign webforms.
 
    // Add intro fieldset.
    $joined_component_id = 1;
    $joined_components[$joined_component_id] = array(
      'pid' => 0,
      'form_key' => 'intro',
      'name' => 'Intro',
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
    $parent = $joined_component_id;
    $joined_component_id ++;

    // Adding intro components');
    foreach ($intro_components as $intro_cid => $intro_component) {
      // Unset unknown id's.
      unset($intro_component['nid']);
      unset($intro_component['cid']);
      $intro_component['pid'] = $parent;
      $intro_component['weight'] = $joined_component_id;
      $joined_components[$joined_component_id] = $intro_component;
      $intro_component_id_map[$intro_cid] = $joined_component_id;
      $joined_component_id ++;
    }

    // Adding intro conditionals.
    $joined_conditional_id = 0;
    foreach ($intro_conditionals as $intro_cid => $intro_conditional) {
      // Unset unknown id's.
      unset($intro_conditional['nid']);
      $intro_conditional['rgid'] = $joined_conditional_id;
      foreach ($intro_conditional['rules'] as $rid => &$rule) {
        unset($rule['nid']);
        unset($rule['rid']);
        $rule['rgid'] = $joined_conditional_id;
        $rule['source'] = $intro_component_id_map[$rule['source']];
      }
      foreach ($intro_conditional['actions'] as $aid => &$rule) {
        unset($rule['nid']);
        unset($rule['aid']);
        $rule['rgid'] = $joined_conditional_id;
        $rule['target'] = $intro_component_id_map[$rule['target']];
      }
      $joined_conditionals[$joined_conditional_id] = $intro_conditional;
      $joined_conditional_id ++;
    }

    // Add intro conditional to show campaign fieldset.
    
    $joined_conditionals[$joined_conditional_id] = array(
      'rgid' => $joined_conditional_id,
      'andor' => NULL,
      'rules' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'source_type' => 'component',
          'source' => $intro_component_id_map[$campaign->condition_component],
          'operator' => 'equal',
          'value' => $campaign->condition_accept_value,
        ),
      ),
      'actions' => array(
        0 => array(
          'rgid' => $joined_conditional_id,
          'target_type' => 'component',
          'target' => $joined_component_id,
          'invert' => 0,
          'action' => 'show',
          'argument' => '',
        ),
      ),
    );
    $joined_conditional_id ++;

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

//    dpm('intro');
//    dpm($intro_components);
//    dpm($intro_conditionals);
//    dpm('campaign');
//    dpm($campaign_components);
//    dpm($campaign_conditionals);
//    dpm('joined');
//    dpm($joined_components);
//    dpm($joined_conditionals);
    // Create new webform.
    $joined_form = new stdClass();
    $joined_form->type = 'webform';
    node_object_prepare($joined_form);
    $joined_form->title = 'Test: '.$campaign_webform->title;
    $joined_form->language = $campaign_webform->language;
    $joined_form->uid = $campaign_webform->uid;
    $joined_form->promote = $campaign_webform->promote;
    $joined_form->comment = $campaign_webform->comment;
    // Attach the webform to the node.
    $joined_form->webform = array(
      'confirmation' => '',
      'confirmation_format' => NULL,
      'redirect_url' => '',
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
    );
    dpm($joined_form);
    node_save($joined_form);
  }
  else {
    dpm('Error');
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
