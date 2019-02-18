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
  );
  return theme('spbellen_call_button', $variables);
}
