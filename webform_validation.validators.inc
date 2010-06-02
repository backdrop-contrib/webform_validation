<?php
// $Id$

/**
 * @file
 * Provides validation functionality and hooks
 */

/**
 * Implementation of hook_webform_validation_validators().
 *
 * This function returns an array of validators, in the validator key => options array form.
 * Possible options:
 * - name (required): name of the validator
 * - component types (required): defines which component types can be validated by this validator. Specify 'all' to allow all types
 * - custom_error (optional): define whether a user can specify a custom error message upon creating the validation rule.
 * - custom_data (optional): define whether custom data can be added to the validation rule
 * - min_components (optional): define the minimum number of components to be selected for creating a validation rule
 * - max_components (optional): define the maximum number of components to be selected for creating a validation rule
 *
 */
function webform_validation_webform_validation_validators() {
  return array(
    'numeric' => array(
      'name' => "Numeric values",
      'component_types' => array(
        'textfield',
      ),
    ),
    'min_length' => array(
      'name' => "Minimum length",
      'component_types' => array(
        'textfield',
        'textarea',
        'email',
      ),
      'custom_data' => array(
        'label' => t('Minimum number of characters'),
        'description' => t('Specify the minimum number of characters that have to be entered to pass validation.')
      ),
    ),
    'max_length' => array(
      'name' => "Maximum length",
      'component_types' => array(
        'textfield',
        'textarea',
        'email',
      ),
      'custom_data' => array(
        'label' => t('Maximum number of characters'),
        'description' => t('Specify the maximum number of characters that can be entered to pass validation.')
      ),
    ),
    'equal' => array(
      'name' => "Equal values",
      'component_types' => array(
        'textfield',
        'email',
        'select',
      ),
      'min_components' => 2,
    ),
    'unique' => array(
      'name' => "Unique values",
      'component_types' => array(
        'textfield',
        'email',
        'select',
      ),
      'min_components' => 2,
    ),
    'oneoftwo' => array(
      'name' => "Require at least one of two fields",
      'component_types' => array(
        'textfield',
        'textarea',
        'email',
        'select',
      ),
      'min_components' => 2,
      'max_components' => 2,
    ),
    'oneofseveral' => array(
      'name' => "Require at least one of several fields",
      'component_types' => array(
        'textfield',
        'textarea',
        'email',
        'select',
      ),
      'min_components' => 2,
    ),
    'select_min' => array(
      'name' => "Minimum number of selections required",
      'component_types' => array(
        'select',
      ),
      'custom_data' => array(
        'label' => t('Minimum number of selections'),
        'description' => t('Specify the minimum number of options a user should select.')
      ),
    ),
    'select_max' => array(
      'name' => "Maximum number of selections allowed",
      'component_types' => array(
        'select',
      ),
      'custom_data' => array(
        'label' => t('Maximum number of selections'),
        'description' => t('Specify the maximum number of options a user can select.')
      ),
    ),
    'select_exact' => array(
      'name' => "Exact number of selections required",
      'component_types' => array(
        'select',
      ),
      'custom_data' => array(
        'label' => t('Number of selections'),
        'description' => t('Specify how many options a user can select.')
      ),
    ),
    'plain_text' => array(
      'name' => "Plain text (disallow tags)",
      'component_types' => array(
        'textfield',
        'textarea',
        'email',
      ),
    ),
    'regex' => array(
      'name' => "Regular expression",
      'component_types' => array(
        'textfield',
        'textarea',
        'email',
      ),
      'custom_error' => TRUE,
      'custom_data' => array(
        'label' => t('Regex code'),
        'description' => t('Specify regex code to validate the user input against.')
      ),
    ),
    'must_be_empty' => array(
      'name' => "Must be empty (Anti-Spam: Hide with CSS)",
      'component_types' => array(
        'textfield',
      ),
    ),
    'blacklist' => array(
      'name' => "Words blacklist",
      'component_types' => array(
        'textfield',
        'textarea',
        'email',
      ),
      'custom_error' => TRUE,
      'custom_data' => array(
        'label' => t('Blacklisted words'),
        'description' => t('Specify illegal words, seperated by commas')
      ),
    ),
  );
}

/**
 * Implementation of hook_webform_validation_validate().
 */
function webform_validation_webform_validation_validate($validator_name, $items, $components, $rule) {
  if ($items) {
    switch ($validator_name) {
      case 'numeric':
        foreach ($items as $key => $val) {
          if ($val && (!is_numeric($val))) {
            $errors[$key] = t('%item is not numeric', array('%item' => $components[$key]['name']));
          }
        }
        return $errors;
        break;
      case 'min_length':
        $min_length = $rule['data'];
        foreach ($items as $key => $val) {
          if ($val && (drupal_strlen($val) < $min_length)) {
            $errors[$key] = t('%item needs to be at least %num characters long', array('%item' => $components[$key]['name'], '%num' => $min_length));
          }
        }
        return $errors;
        break;
      case 'max_length':
        $max_length = $rule['data'];
        foreach ($items as $key => $val) {
          if ($val && (drupal_strlen($val) > $max_length)) {
            $errors[$key] = t('%item can be maximum %num characters long', array('%item' => $components[$key]['name'], '%num' => $max_length));
          }
        }
        return $errors;
        break;
      case "equal":
        $first_entry_key = key($items);
        $first_entry = array_shift($items);
        $first_entry = _webform_validation_flatten_array($first_entry); // flatten in case of array
        // now check if following components equal the first one
        foreach ($items as $key => $val) {
          $val = _webform_validation_flatten_array($val); // flatten in case of array
          if ($val !== $first_entry) {
            $errors[$key] = t('%item_checked does not match %item_first', array('%item_checked' => $components[$key]['name'], '%item_first' => $components[$first_entry_key]['name']));
          }
        }
        return $errors;
        break;
      case "unique":
        foreach ($items as $key => $val) {
          if (is_array($val)) {
            // make sure to flatten arrays first
            $items[$key] = _webform_validation_flatten_array($val);
          }
          if (empty($items[$key])) {
            // items without a value selected shouldn't be validated
            unset($items[$key]);
          }
        }
        // now we count how many times each value appears, and find out which values appear more than once
        $items_count = array_count_values(array_map('strtolower', array_map('trim', $items)));
        $doubles = array_filter($items_count, create_function('$x', 'return $x > 1;'));
        foreach ($items as $key => $val) {
          if (in_array($val, array_keys($doubles))) {
            $errors[$key] = t('The value of %item is not unique', array('%item' => $components[$key]['name']));
          }
        }
        return $errors;
        break;
      case "oneoftwo":
        // $components should have 2 items
        $keys = array_keys($items);
        $item1 = array_shift($keys);
        $item2 = array_shift($keys);
        $entry1 = _webform_validation_flatten_array($items[$item1]);
        $entry2 = _webform_validation_flatten_array($items[$item2]);
        if (empty($entry1) && empty($entry2)) {
          return array($item1 => t('You have to specify %item1 or %item2 (or both)', array('%item1' => $components[$item1]['name'], '%item2' => $components[$item2]['name'])));
        }
        break;
      case "oneofseveral":
        foreach ($items as $key => $val) {
          if (is_array($val)) {
            // make sure to flatten arrays first
            $items[$key] = _webform_validation_flatten_array($val);
          }
        }
        // $components should have at least one of several items
        if (count(array_filter($items)) < 1) {
          $keys = array_keys($items);
          $names = array();
          foreach ($keys as $value) {
            $names[] = $components[$value]['name'];
          }
          return array($keys[0] => t('You have to specify at least one of these items:') . theme('item_list', $names));
        }
        break;
      case "select_min":
        $min_selections = $rule['data'];
        foreach ($items as $key => $val) {
          if (is_array($val) && (count(array_filter($val)) < $min_selections)) {
            $errors[$key] = t('Please select at least %num options for %item', array('%num' => $min_selections, '%item' => $components[$key]['name']));
          }
        }
        return $errors;
        break;
      case "select_max":
        $max_selections = $rule['data'];
        foreach ($items as $key => $val) {
          if (is_array($val) && (count(array_filter($val)) > $max_selections)) {
            $errors[$key] = t('Please select maximum %num options for %item', array('%num' => $max_selections, '%item' => $components[$key]['name']));
          }
        }
        return $errors;
        break;
      case "select_exact":
        $allowed_selections = $rule['data'];
        foreach ($items as $key => $val) {
          if (is_array($val) && (count(array_filter($val)) != $allowed_selections)) {
            $errors[$key] = t('Please select %num options for %item', array('%num' => $allowed_selections, '%item' => $components[$key]['name']));
          }
        }
        return $errors;
        break;
      case "plain_text":
        foreach ($items as $key => $val) {
          if ($val && (strcmp($val, strip_tags($val)))) {
            $errors[$key] = t('%item only allows the use of plain text', array('%item' => $components[$key]['name']));
          }
        }
        return $errors;
        break;
      case "regex":
        $regex = $rule['data'];
        foreach ($items as $key => $val) {
          if ($val && (!preg_match("/$regex/", $val))) {
            $errors[$key] = _webform_validation_i18n_error_message($rule);
          }
        }
        return $errors;
        break;
      case 'must_be_empty':
        foreach ($items as $key => $val) {
          if ($val) {
            $errors[$key] = t('%item does not contain the correct data', array('%item' => $components[$key]['name']));
          }
        }
        return $errors;
        break;
      case "blacklist":
          $blacklist = explode(',', $rule['data']);
          $blacklist = array_map('trim', $blacklist);
          $blacklist_regex = implode('|', $blacklist);
          foreach ($items as $key => $val) {
            if ($val && (preg_match("/$blacklist_regex/i", $val))) {
              $errors[$key] = _webform_validation_i18n_error_message($rule);
            }
          }
        return $errors;
        break;
    }
  }
}

/**
 * Helper function to deal with submitted values that are arrays (e.g. multiple select component)
 * We flatten the array as a comma-separated list to do the comparison.
 */
function _webform_validation_flatten_array($val) {
  if (is_array($val)) {
    $arr = array_filter($val);
    return implode(',', $arr);
  }
  return $val;
}

/**
 * Get a list of validator definitions
 */
function webform_validation_get_validators() {
  $validators = module_invoke_all("webform_validation_validators");
  // let modules use hook_webform_validator_alter($validators) to change validator settings
  drupal_alter('webform_validator', $validators);
  return $validators;
}

function webform_validation_get_validators_selection() {
  $selection = array();
  $validators = webform_validation_get_validators();
  if ($validators) {
    foreach ($validators as $validator_key => $validator_info) {
      $selection[$validator_key] = $validator_info['name'];
    }
  }
  return $selection;
}

/**
 * Get a list of valid component types per validator, as defined via hook_webform_validation_validators().
 * If 'all' is specified, all available component types will be returned.
 */
function webform_validation_valid_component_types($validator) {
  $validators = webform_validation_get_validators();
  if ($info = $validators[$validator]) {
    $allowed_types = $info['component_types'];
    if (_webform_validation_all_allowed($allowed_types)) {
      return webform_load_components();
    }
    return $info['component_types'];
  }
}

/**
 * Helper function to check whether all components are allowed to be used for a certain validator
 */
function _webform_validation_all_allowed($allowed) {
  if ($allowed) {
    foreach ($allowed as $type) {
      if ($type == "all") {
        return TRUE;
      }
    }
  }
  return FALSE;
}

function webform_validation_get_validator_info($validator_key) {
  $validators = webform_validation_get_validators();
  return $validators[$validator_key];
}

/**
 * Handle translatable error messages, if available
 */
function _webform_validation_i18n_error_message($rule) {
  if (module_exists('i18nstrings')) {
    return i18nstrings('webform_validation:error_message:' . $rule['ruleid'] . ':message', $rule['error_message']);
  }
  return $rule['error_message'];
}