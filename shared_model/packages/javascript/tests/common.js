/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

const test = require('tape')
const { ERROR_MESSAGES } = require('./helpers')
const { 
  Blob, 
  Hash, 
  ByteVector,
  StringVector,
  HashVector,
  Role,
  RolePermissionSet
} = require('../index.js')

test('Common unit tests', function (t) {
  t.plan(13)
  
  const hexString = '11111111111111111111111111111111'

  t.comment('Blob tests')
  t.throws(() => new Blob(), ERROR_MESSAGES.INVALID_CONSTRUCTOR_PARAMETERS, 'Should throw illegal number of arguments')
  t.ok(new Blob('') instanceof Blob, 'Should be an instance of Blob')
  
  const blob = new Blob(hexString)
  t.ok(blob.blob() instanceof ByteVector, 'Should be an instance of ByteVector')
  t.equals(blob.hex(), '3131313131313131313131313131313131313131313131313131313131313131', 'Should be the same as hex representation')
  t.equals(blob.size(), hexString.length, 'Should have the same as length of hexString')

  t.comment('Hash tests')
  t.throws(() => new Hash(), ERROR_MESSAGES.INVALID_CONSTRUCTOR_PARAMETERS, 'Should throw illegal number of arguments')
  t.ok(new Hash('') instanceof Hash, 'Should be an instance of Hash')

  const hash = new Hash(hexString)
  t.equals(hash.hex(), '3131313131313131313131313131313131313131313131313131313131313131', 'Should be the same as hex representation')
  t.equals(hash.toString(), 'Hash: [3131313131313131313131313131313131313131313131313131313131313131 ]', 'Should be in the format of log')

  t.comment('HashVector tests')
  t.ok(new HashVector() instanceof HashVector, 'Should have the same type as HashVector')

  let hv = new HashVector()
  t.doesNotThrow(() => hv.push_back(hash), null, 'Should add new member to the vector')
  t.equals(hv.get(0).hex(), hash.hex(), 'Should return the first member of the vector')
  t.equals(hv.size(), 1, 'Should have size = 1')

  t.end()
})

test('Permissions tests', function (t) {
  t.plan(6)
  
  t.comment('RolePermissionSet tests')
  t.doesNotThrow(() => new RolePermissionSet(), null, 'Should be presented as a class')

  const perms = new RolePermissionSet()
  t.equals(perms.size(), 43, 'Should have size 43')
  t.throws(() => perms.set(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw ILLEGAL_NUMBER_OF_ARGUMENTS')
  // TODO: Type checking doesn't work with enums, so we need to add checkers in library wrapper
  // t.throws(() => perms.set(''), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw WRONG_ARGUMENT_TYPE')
  // t.throws(() => perms.set(1), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw WRONG_ARGUMENT_TYPE')

  // This test mutate perms object, so tests below depends on it
  t.doesNotThrow(() => perms.set(Role.kAppendRole), null, 'Should add kAppendRole to the set')

  t.equals(perms.test(Role.kAppendRole), true, 'Should have kAppendRole permission in the set')
  t.equals(perms.none(), false, 'Shouldn`t be empty set')

  t.end()
})
