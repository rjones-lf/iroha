/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

var test = require('tape')
const { ERROR_MESSAGES } = require('./helpers')
var iroha = require('../index')

const accountId = 'admin@test'
const assetId = 'coin#test'

test('ModelQueryBuilder tests', function (t) {
  t.plan(49)

  let queryBuilder = new iroha.ModelQueryBuilder()
  const time = Date.now()

  // Tests for concrete query
  t.comment('Basic QueryBuilder tests')
  t.throws(() => queryBuilder.build(), /Missing concrete query/, 'Should throw Missing concrete query')
  t.throws(() => queryBuilder.creatorAccountId(accountId).build(), /Missing concrete query/, 'Should throw Missing concrete query')
  t.throws(() => queryBuilder.creatorAccountId(accountId).createdTime(time).build(), /Missing concrete query/, 'Should throw Missing concrete query')
  t.throws(() => queryBuilder.creatorAccountId(accountId).createdTime(time).queryCounter(1).build(), /Missing concrete query/, 'Should throw Missing concrete query')
  t.throws(() => queryBuilder.creatorAccountId('').createdTime(time).queryCounter(1).getAccount(accountId).build(), /Wrongly formed creator_account_id, passed value: ''/, 'Should throw Wrongly formed creator_account_id')
  t.throws(() => queryBuilder.creatorAccountId(accountId).createdTime().build(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => queryBuilder.creatorAccountId(accountId).createdTime('').build(), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw ...argument 1 of type...')
  t.throws(() => queryBuilder.creatorAccountId(accountId).createdTime(0).queryCounter(1).getAccount(accountId).build(), /bad timestamp: too old, timestamp: 0, now:/, 'Should throw bad timestamp: too old')
  t.throws(() => queryBuilder.creatorAccountId(accountId).createdTime(time).queryCounter(0).getAccount(accountId).build(), /Counter should be > 0, passed value: 0/, 'Should throw Counter should be > 0')

  // Query with valid queryCounter, creatorAccountId and createdTime
  let correctQuery = queryBuilder.creatorAccountId(accountId).createdTime(time).queryCounter(1)

  // getAccount() tests
  t.comment('Testing getAccount()')
  t.throws(() => correctQuery.getAccount(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccount('').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctQuery.getAccount('@@@').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.doesNotThrow(() => correctQuery.getAccount(accountId).build(), null, 'Should not throw any exceptions')

  // getSignatories() tests
  t.comment('Testing getSignatories()')
  t.throws(() => correctQuery.getSignatories(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getSignatories('').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctQuery.getSignatories('@@@').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.doesNotThrow(() => correctQuery.getSignatories(accountId).build(), null, 'Should not throw any exceptions')

  // getAccountTransactions() tests
  t.comment('Testing getAccountTransactions()')
  t.throws(() => correctQuery.getAccountTransactions(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccountTransactions('').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctQuery.getAccountTransactions('@@@').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.doesNotThrow(() => correctQuery.getAccountTransactions(accountId).build(), null, 'Should not throw any exceptions')

  // getAccountAssetTransactions() tests
  t.comment('Testing getAccountAssetTransactions()')
  t.throws(() => correctQuery.getAccountAssetTransactions(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccountAssetTransactions(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccountAssetTransactions('', assetId).build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctQuery.getAccountAssetTransactions('@@@', assetId).build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctQuery.getAccountAssetTransactions(accountId, '').build(), /Wrongly formed asset_id, passed value: ''/, 'Should throw Wrongly formed asset_id')
  t.throws(() => correctQuery.getAccountAssetTransactions(accountId, '@@@').build(), /Wrongly formed asset_id, passed value: '@@@'/, 'Should throw Wrongly formed asset_id')
  t.doesNotThrow(() => correctQuery.getAccountAssetTransactions(accountId, assetId).build(), null, 'Should not throw any exceptions')

  // getAccountAssets() tests
  t.comment('Testing getAccountAssets()')
  t.throws(() => correctQuery.getAccountAssets(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw ILLEGAL_NUMBER_OF_ARGUMENTS')
  t.throws(() => correctQuery.getAccountAssets('').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctQuery.getAccountAssets('@@@').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.doesNotThrow(() => correctQuery.getAccountAssets(accountId).build(), null, 'Should not throw any exceptions')

  // getRoles() tests
  t.comment('Testing getRoles()')
  t.doesNotThrow(() => correctQuery.getRoles().build(), null, 'Should not throw any exceptions')

  // getAssetInfo() tests
  t.comment('Testing getAssetInfo()')
  t.throws(() => correctQuery.getAssetInfo(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAssetInfo('').build(), /Wrongly formed asset_id, passed value: ''/, 'Should throw Wrongly formed asset_id')
  t.throws(() => correctQuery.getAssetInfo('@@@').build(), /Wrongly formed asset_id, passed value: '@@@'/, 'Should throw Wrongly formed asset_id')
  t.doesNotThrow(() => correctQuery.getAssetInfo(assetId).build(), null, 'Should not throw any exceptions')

  // getRolePermissions() tests
  t.comment('Testing getRolePermissions()')
  t.throws(() => correctQuery.getRolePermissions(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getRolePermissions('').build(), /Wrongly formed role_id, passed value: ''/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctQuery.getRolePermissions('@@@').build(), /Wrongly formed role_id, passed value: '@@@'/, 'Should throw Wrongly formed role_id')
  t.doesNotThrow(() => correctQuery.getRolePermissions('role').build(), null, 'Should not throw any exceptions')

  // getTransactions() tests
  t.comment('Testing getTransactions()')
  t.throws(() => correctQuery.getTransactions(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getTransactions(''), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw ...argument 2 of type...')

  let hv = new iroha.HashVector()
  hv.push_back(new iroha.Hash('11111111111111111111111111111111'))
  hv.push_back(new iroha.Hash('22222222222222222222222222222222'))
  let emptyHv = new iroha.HashVector()

  t.throws(() => correctQuery.getTransactions(emptyHv), /Hash set should contain at least one hash/, 'Should throw Hash set should contain at least one hash')
  t.doesNotThrow(() => correctQuery.getTransactions(hv), null, 'Should not throw any exceptions')

  // getAccountDetail() tests
  t.comment('Testing getAccountDetail()')
  t.throws(() => correctQuery.getAccountDetail(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccountDetail('').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctQuery.getAccountDetail('@@@').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.doesNotThrow(() => correctQuery.getAccountDetail(accountId).build(), null, 'Should not throw any exceptions')

  t.end()
})
