<?php

/**
 * @file
 * Add call butoon page.
 */

/**
 * Returns call button page for logged in caller.
 */
function spbellen_call_button() {
  global $user;
  $contact_data = spbellen_get_caller_target_contact_data($user->uid);

  $variables = array(
    'phone_number' => empty($contact_data['phone']) ? '' : $contact_data['phone'],
    'display_name' => empty($contact_data['display_name']) ? '' : $contact_data['display_name'],
  );
  return theme('spbellen_call_button', $variables);
}
