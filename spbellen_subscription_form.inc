<?php

function spbellen_subscription_form($form, &$form_state) {

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => 'Naam',
  );

  $form['phone'] = array(
    '#type' => 'textfield',
    '#title' => '06-nummer',
  );

  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => 'E-mail',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Indienen',
    '#name' => 'submit',
  );
  return $form;
}
