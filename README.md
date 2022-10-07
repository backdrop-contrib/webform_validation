# Webform Validation

This module adds an extra tab to each webform node, allowing you to specify
validation rules for your webform components. You can create one or more of the
predefined validation rules, and select which webform component(s) should be
validated against those. By using the hooks provided by this module, you can
also define your own validation rules in your own modules.

The following validation rules are currently included:

- Numeric values (optionally specify min and / or max value)
- Minimum length
- Maximum length (automatically integrates a JavaScript counter if the optional
  Maxlength module is installed)
- Minimum number of words
- Maximum number of words
- Equal values on multiple fields
- Unique values on multiple fields
- Specific value
- Require at least one of two fields
- Require at least one of several fields
- Minimum number of selections required
- Maximum number of selections allowed
- Exact number of selections required
- Plain text (disallow tags)
- Regular expression
- Must be empty (Anti-Spam: Hide with CSS)
- Words blacklist
- Must match a username

## Installation

 - Make sure you have the Webform module enabled
 - Install this module using the official
  [Backdrop CMS instructions](https://backdropcms.org/guide/modules)

## Usage

Instructions can be viewed and edited in the
[Wiki](https://github.com/backdrop-contrib/webform_validation/wiki).

## Issues

Bugs and Feature requests should be reported in the
[Issue Queue](https://github.com/backdrop-contrib/webform_validation/issues)

## Current Maintainers

- [Laryn Kragt Bakker](https://github.com/laryn) - [CEDC.org](https://cedc.org)

## Credits

- Ported to Backdrop CMS by [Laryn Kragt Bakker](https://github.com/laryn) - [CEDC.org](https://cedc.org).
- Current maintainer for the Drupal module: [Liam Morland](https://github.com/lkmorlan)
- Original author of the Drupal module: [Sven Decabooter](https://github.com/svendecabooter)
- The MaxLength integration is based on work by
  [Hal Eagar](https://www.drupal.org/node/1459650#comment-9011799),
  with enhancements from [Allen Shaw](https://github.com/twomice).

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for
complete text.
