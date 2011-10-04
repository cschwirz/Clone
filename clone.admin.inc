<?php

/**
 * @file
 * Administration page callbacks for the myfile module.
 */

/**
 * Form builder. Configure Clone.
 */
function clone_admin_settings() {
  $form['clone_server_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Server address'),
    '#default_value' => variable_get('clone_server_address'),
    '#description' => t('The public address of this drupal installation'),
    '#size' => 40,
    '#maxlength' => 256,
    '#required' => TRUE,
  );
  $form['clone_allow_clone'] = array(
    '#type' => 'checkbox',
    '#title' => t('Allow clone requests.'),
    '#default_value' => variable_get('clone_allow_clone', FALSE),
  );
  $form['#submit'][] = 'clone_admin_settings_submit'; // XXX
  return system_settings_form($form);
}

/**
 * Process Clone settings submission
 */
function clone_admin_settings_submit($form, &$form_state) {
  watchdog('Clone', 'admin settings submitted');
  variable_set('clone_server_address',
    $form_state['values']['clone_server_address']);
  variable_set('clone_allow_cloning',
    $form_state['values']['clone_allow_clone']);
}
