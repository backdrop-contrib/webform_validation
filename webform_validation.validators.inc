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
      ),
      'min_components' => 2,
    ),
    'unique' => array(
      'name' => "Unique values",
      'component_types' => array(
        'textfield',
        'email',
      ),
      'min_components' => 2,
    ),
    'oneoftwo' => array(
      'name' => "Require at least one of two fields",
      'component_types' => array(
        'textfield',
        'textarea',
        'email',
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
      ),
      'min_components' => 2,
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
        // now check if following components equal the first one
        foreach ($items as $key => $val) {
          if ($val !== $first_entry) {
            $errors[$key] = t('%item_checked does not match %item_first', array('%item_checked' => $components[$key]['name'], '%item_first' => $components[$first_entry_key]['name']));
          }
        }
        return $errors;
        break;
      case "unique":
        $items_count = array_count_values($items);
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
        if (empty($items[$item1]) && empty($items[$item2])) {
          return array($item1 => t('You have to specify %item1 or %item2 (or both)', array('%item1' => $components[$item1]['name'], '%item2' => $components[$item2]['name'])));
        }
        break;
      case "oneofseveral":
        // $components should have at least one of several items
        if (count(array_filter($items)) < 1) {
          $keys = array_keys($items);
          $names = array();
          foreach($keys as $value) {
            $names[] = $components[$value]['name'];
          }
          return array($keys[0] => t('You have to specify at least one of these items:') . theme('item_list', $names));
        }
        break;
      case "regex":
        $regex = $rule['data'];
        foreach ($items as $key => $val) {
          if ($val && (!preg_match("/$regex/", $val))) {
            $errors[$key] = $rule['error_message'];
          }
        }
        return $errors;
        break;
    }
  }
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