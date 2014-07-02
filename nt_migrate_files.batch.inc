<?php
/**
 * @file
 * nt_migrate_files.batch.inc
 * @copyright The Royal National Theatre
 * @author John-Paul Drawneek
 */

/**
 * S3 upload form.
 *
 * @param array $form
 *   Current for structure.
 * @param array $form_state
 *   Current form state.
 *
 * @return array
 *   Form structure to render.
 */
function nt_migrate_files_upload_form($form, &$form_state) {
  form_load_include($form_state, 'inc', 'nt_migrate_files', 'nt_migrate_files.batch');
  $form = array();
  $form['full'] = array(
    '#type'  => 'fieldset',
    '#title' => t('Full upload'),
  );
  $form['full']['all'] = array(
    '#type'     => 'submit',
    '#value'    => t('Go'),
    '#submit'   => array('nt_migrate_files_upload_all'),
  );
}
/**
 * S3 upload all file batch setup.
 *
 * @param array $form
 *   Current for structure.
 * @param array $form_state
 *   Current form state.
 */
function nt_migrate_files_upload_all($form, &$form_state) {
  $batch = array(
    'title' => t('Upload Files'),
    'operations' => array(
      array('_nt_migrate_files_upload_parse', array()),
    ),
    'finished' => 'nt_migrate_files_refresh_batch_fin',
    'file' => drupal_get_path('module', 'nt_migrate_files') . '/nt_migrate_files.batch.inc',
  );
  batch_set($batch);
}
/**
 * Batch process end function.
 *
 * @param boolean $success
 *   Flag to say if it was successful.
 * @param array $results
 *   Results passed back from batch functions.
 * @param string $operations
 *   Not sure.
 */
function nt_migrate_files_refresh_batch_fin($success, $results, $operations) {
  if ($success) {
    drupal_set_message('Files upload to S3 bucket.');
  }
}
/**
 * Batch job to upload files to S3.
 * @access private
 *
 * @param array $context
 *   Current batch config.
 */
function _nt_migrate_files_upload_parse(&$context) {
  if (empty($context['sandbox'])) {
    $context['sandbox'] = array();
    $result = db_query('SELECT fid FROM {file_managed}');
    $context['sandbox']['fids'] = $result->fetchCol();
  }
  $fid = array_shift($context['sandbox']['fids']);
  $file = file_load($fid);
  $uri = $file->uri;
  if (strpos($uri, 's3://') === FALSE) {
    $destination = 's3://' . file_uri_target($uri);
    file_move($file, $destination, FILE_EXISTS_RENAME);
  }
  $context['message'] = t(
    'Processing "@one" out of "@two"',
    array(
      '@one' => count($context['sandbox']['fids']),
      '@two' => $context['sandbox']['max'],
    )
  );
  if (count($context['sandbox']['fids']) > 0) {
    $context['finished']
      = ($context['sandbox']['max'] - count($context['sandbox']['fids']))
            / $context['sandbox']['max'];
  }
}
