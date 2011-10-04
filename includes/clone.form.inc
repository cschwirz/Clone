<?php

/**
 * Give a list of cloneable patterns.
 *
 * TODO: use public flag of pattern?
 *       allow cloneable patterns to be select in an admin menu?
 */
function clone_clone_form() {
  $header = array('Title', 'Version', 'Name', 'Category');
  $patterns = patterns_get_patterns();
  $rows = array();
  foreach ($patterns as $pid => $pattern) {
    $category = $pattern->info['category'] ? $pattern->info['category'] : t('Other');
    $row = array($pattern->title, @$pattern->info['version'], $pattern->name, $category); //, l(t('View'), 'clone/show/' . $pid));
    $rows['pid' . $pid] = $row;
  }
  $form['clone_table'] = array
  (
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('No cloneable patterns found.'),
  );
  $form['clone_destination_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Destination address'),
    '#description' => t('The address of the instance you want to clone to.'),
    '#size' => 40,
    '#maxlength' => 256,
    '#required' => TRUE,
  );
  $form['submit'] = array(
    '#type' => 'submit', 
    '#value' => 'Submit',
  );
  return $form;
}

/**
 * List open clone requests.
 *
 * List patterns that have been sent from another instance and have not yet been processed.
 * Pattern can be added to own patterns or delete.
 */
function clone_request_form($form, $form_state) {
  $header = array('ID', 'URL', 'IP', 'Time', 'Name', 'Title', 'Version', 'Format');
  $rows = array();
  $result = db_query('SELECT id, url, ip, created, name, title, version, format, content FROM {clone_requests}');
  foreach ($result as $record) {
    $row = array($record->id, $record->url, $record->ip, format_date($record->created), $record->name, $record->title, $record->version, $record->format);
    $rows[$record->id] = $row;
  }
  $form['request_table'] = array
  (
    '#type' => 'tableselect',
    '#header' => $header,
    '#options' => $rows,
    '#empty' => t('No open requests listed.'),
  );
  $form['filename_prefix'] = array(
    '#type' => 'textfield',
    '#description' => t('Prefix prepended to the filename.'),
    '#size' => 32,
    '#maxlength' => 32,
    '#required' => FALSE,
  );
  $form['submit_type'] = array(
    '#type' => 'select',
    '#default_value' => 'add',
    '#options' => array(
      'add' => t('Add to Patterns'),
      'delete' => t('Delete'),
    ),
  );
  $form['submit'] = array
  (
    '#type' => 'submit',
    '#value' => t('Submit'),
  );
  return $form;
}

/**
 * Process the form of open clone requests.
 */
function clone_request_form_submit($form, &$form_state) {
  $add_to_patterns = FALSE;
  if ($form_state['values']['submit_type'] == 'add') {
    $add_to_patterns = TRUE;
  }
  $result = db_query('SELECT id, name, format, content FROM {clone_requests}');
  foreach ($result as $record) {
    if ($form_state['values']['request_table'][$record->id]) {
      $success = TRUE;
      if ($add_to_patterns) {
        $success = FALSE;
        $prefix = $form_state['values']['filename_prefix'];
        $success = patterns_io_save_pattern($record->content, $prefix . $record->name, $record->format);
      }
      if ($success) {
        db_delete('clone_requests')->condition('id', $record->id)->execute();
      }
    }
  }
}

/**
 * Process form releated to cloneable patterns.
 */
function clone_clone_form_submit($form, &$form_state) {
  $patterns = patterns_get_patterns();
  $clone_patterns = array();
  $n_clone_patterns = 0;
  foreach ($patterns as $pid => $pattern) {
    if ($form_state['values']['clone_table']['pid' . $pid]) {
      if ($pattern->file) {
        $name = $pattern->name;
        $title = $pattern->title;
        $content = file_get_contents($pattern->file);
        $format = 'yaml'; // TODO
        $valid_extensions = array('xml', 'yaml'); // TODO allow different extensions?
        foreach ($valid_extensions as $extension) {
          if (strrpos($pattern->file, $extension) === strlen($pattern->file) - strlen($extension)) {
            $format = $extension;
          }
        }
        $version = ''; // TODO: NULL as default?
        if (array_key_exists('version', $pattern->info)) {
          $version = $pattern->info['version'];
        }
        $clone_patterns[] = array(
          'name' => $name,
          'title' => $title,
          'version' => $version,
          'format' => $format,
          'content' => $content,
        );
        $n_clone_patterns++;
      }
    }
  }
  if ($n_clone_patterns === 0) {
      drupal_set_message(t('Error: no valid pattern selected.'));
  }
  else {
    $base_url = $form_state['values']['clone_destination_address'];
    $server = url($base_url . "/xmlrpc.php");
    $myurl = variable_get('clone_server_address');
    $result = xmlrpc(
        $server,
        array(
          'clone.addCloneRequest' =>
          array($myurl, ip_address(), $clone_patterns),
          )
        );
    if ($result === FALSE) {
      drupal_set_message(t("Error return from xmlrpc(): Error: @errno, Message: @message", array('@errno' => xmlrpc_errno(), '@message' => xmlrpc_error_msg())));
    }
    else {
      drupal_set_message(t("The XMLRPC server returned this response: @response", array('@response' => print_r($result, TRUE))));
    }
  }
}
