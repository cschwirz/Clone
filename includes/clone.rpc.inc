<?php

/**
 * Implements hook_xmlrpc().
 */
function clone_xmlrpc() {
  $methods['clone.hello'] = 'xmls_clone_hello';
  $methods['clone.addCloneRequest'] = 'xmls_clone_addrequest';
  return $methods;
}

/**
 * Add pattern from request to database.
 */
function xmls_clone_addrequest($url, $ip, $patterns) {
  if ($ip) {
    watchdog('Clone', 'xmls_clone_addrequest ' . $ip);
  }
  if (!$url) {
    return xmlrpc_error(1, t('error adding url to the list'));
  }
  if (variable_get('clone_allow_clone', FALSE)) {
    foreach ($patterns as $pattern) {
      $id = db_insert('clone_requests')
        ->fields(array(
              'url' => $url,
              'ip' => $ip,
              'created' => time(),
              'name' => $pattern['name'],
              'title' => $pattern['title'],
              'version' => $pattern['version'],
              'format' => $pattern['format'],
              'content' => $pattern['content'],
              ))
        ->execute();
      watchdog('Clone', $pattern['title'] . ' ' . $pattern['version']);
    }
    return t('Successfully made a clone request.');
  }
  else {
    return xmlrpc_error(1, t('Sorry, but this instance does not allow any clone requests.'));
  }
}
