<?php

function spbellen_login_form($form, &$form_state) {
  $form['code'] = array(
    '#type' => 'textfield',
    '#title' => 'Code',
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => 'Naam',
  );
  $form['email_id'] = array(
    '#type' => 'textfield',
    '#title' => 'Email of Lidnummer of 06-nummer',
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Nu bellen',
  );
  return $form;
}
