/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * Standard exception messages for Iroha Library.
 * You can export what you want to use in tests.
 */

const ERROR_MESSAGES_SWIG = {
  ILLEGAL_NUMBER_OF_ARGUMENTS: /Error: Illegal number of arguments/,
  WRONG_ARGUMENT_TYPE: /argument \d of type '[\w:><]+'/,
  CANNOT_CONVERT_ARG: /argument \d of type '[\w:><]+'/,
  NUMBER_NOT_IN_RANGE: /argument \d of type '[\w:><]+'/
}

const ERROR_MESSAGES_EMSCRIPTEN = {
  ILLEGAL_NUMBER_OF_ARGUMENTS: /function \w+.\w+ called with \d arguments, expected \d args!/,
  WRONG_ARGUMENT_TYPE: /Cannot pass .+ as a \w+/,
  CANNOT_CONVERT_ARG: /Cannot convert "\w+" to .+/,
  NUMBER_NOT_IN_RANGE: /Passing a number .+/,
  INVALID_CONSTRUCTOR_PARAMETERS: /Tried to invoke ctor of \w+ with invalid number of parameters/
}

module.exports.ERROR_MESSAGES = ERROR_MESSAGES_EMSCRIPTEN
