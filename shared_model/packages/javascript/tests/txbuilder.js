/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

const test = require('tape')
const { ERROR_MESSAGES } = require('./helpers')
const iroha = require('../index')

const publicKey = '407e57f50ca48969b08ba948171bb2435e035d82cec417e18e4a38f5fb113f83'
const privateKey = '1d7e0a32ee0affeb4d22acd73c2c6fb6bd58e266c8c2ce4fa0ffe3dd6a253ffb'

const adminAccountId = 'admin@test'
const assetId = 'coin#test'
const testAccountId = 'test@test'

test('ModelTransactionBuilder tests', function (t) {
  t.plan(134)

  let crypto = new iroha.ModelCrypto()
  let keypair = crypto.convertFromExisting(publicKey, privateKey)

  let txBuilder = new iroha.ModelTransactionBuilder()
  const time = Date.now()
  const futureTime = 2400000000000
  const address = '0.0.0.0:50051'

  t.comment('Basic TransactionBuilder tests')

  t.throws(() => txBuilder.build(), /Transaction should contain at least one command(.*)Wrongly formed creator_account_id, passed value: ''(.*)bad timestamp: too old/, 'Should throw exception 0 commands in transaction, wrong creator_account_id, timestamp')
  t.throws(() => txBuilder.creatorAccountId(adminAccountId).build(), /Transaction should contain at least one command(.*)bad timestamp: too old/, 'Should throw exception about zero commands in transaction, wrong timestamp')
  t.throws(() => txBuilder.creatorAccountId(adminAccountId).createdTime(0).build(), /Transaction should contain at least one command(.*)bad timestamp: too old/, 'Should throw 0 commands + bad timestamp: too old')
  t.throws(() => txBuilder.creatorAccountId(adminAccountId).createdTime(time).build(), /Transaction should contain at least one command/, 'Should throw 0 commands')
  t.throws(() => txBuilder.creatorAccountId('').createdTime(time).build(), /Transaction should contain at least one command(.*)Wrongly formed creator_account_id, passed value: ''/, 'Should throw 0 commands + Wrongly formed creator_account_id')
  t.throws(() => txBuilder.creatorAccountId('@@@').createdTime(time).build(), /Transaction should contain at least one command(.*)Wrongly formed creator_account_id, passed value: '@@@'/, 'Should throw 0 commands + Wrongly formed creator_account_id')
  t.throws(() => txBuilder.creatorAccountId(adminAccountId).createdTime(futureTime).build(), /Transaction should contain at least one command(.*)bad timestamp: sent from future/, 'Should throw exception about zero commands in transaction, Sent from future')
  t.throws(() => txBuilder.creatorAccountId(adminAccountId).createdTime(time).build(), /Transaction should contain at least one command/, 'Should throw exception about zero commands in transaction')
  t.throws(() => txBuilder.quorum(0).build(), /(.*)Quorum should be within range \(0, 128\](.*)/, 'Should throw exception about zero quorum')

  // Transaction with valid creatorAccountId and createdTime
  let correctTx = txBuilder.creatorAccountId(adminAccountId).createdTime(time)

  // addAssetQuantity() tests
  t.comment('Testing addAssetQuantity()')
  t.throws(() => correctTx.addAssetQuantity(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addAssetQuantity(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addAssetQuantity('', ''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addAssetQuantity('', '', '').build(), /AddAssetQuantity: \[\[Wrongly formed account_id, passed value: ''(.*)Wrongly formed asset_id, passed value: ''(.*)Amount must be greater than 0, passed value: 0 \]\]/, 'Should throw wrongly formed account_id, asset_id, Amount must be greater than 0')
  t.throws(() => correctTx.addAssetQuantity(adminAccountId, assetId, '0').build(), /AddAssetQuantity: \[\[Amount must be greater than 0, passed value: 0 \]\]/, 'Should throw Amount must be greater than 0')
  t.throws(() => correctTx.addAssetQuantity('', assetId, '1000').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.addAssetQuantity('@@@', assetId, '1000').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.addAssetQuantity(adminAccountId, '', '1000').build(), /Wrongly formed asset_id, passed value: ''/, 'Should throw Wrongly formed asset_id')
  t.throws(() => correctTx.addAssetQuantity(adminAccountId, '###', '1000').build(), /Wrongly formed asset_id, passed value: '###'/, 'Should throw Wrongly formed asset_id')
  t.doesNotThrow(() => correctTx.addAssetQuantity(adminAccountId, assetId, '1000').build(), null, 'Should not throw any exceptions')

  // addPeer() tests
  t.comment('Testing addPeer()')
  t.throws(() => correctTx.addPeer(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addPeer(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addPeer('', keypair.publicKey()).build(), /Wrongly formed peer address/, 'Should throw exception about wrongly formed peer address')
  t.throws(() => correctTx.addPeer(address, '').build(), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw ...argument 3 of type...')
  t.doesNotThrow(() => correctTx.addPeer(address, keypair.publicKey()).build(), null, 'Should not throw any exceptions')

  // addSignatory() tests
  t.comment('Testing addSignatory()')
  t.throws(() => correctTx.addSignatory(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addSignatory(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addSignatory('', keypair.publicKey()).build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.addSignatory('@@@', keypair.publicKey()).build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.addSignatory(adminAccountId, '').build(), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw ...argument 3 of type...')
  t.doesNotThrow(() => correctTx.addSignatory(adminAccountId, keypair.publicKey()).build(), null, 'Should not throw any exceptions')

  // removeSignatory() tests
  t.comment('Testing removeSignatory()')
  t.throws(() => correctTx.removeSignatory(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.removeSignatory(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.removeSignatory('', keypair.publicKey()).build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.removeSignatory('@@@', keypair.publicKey()).build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.removeSignatory(adminAccountId, '').build(), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw ...argument 3 of type...')
  t.doesNotThrow(() => correctTx.removeSignatory(adminAccountId, keypair.publicKey()).build(), null, 'Should not throw any exceptions')

  // appendRole() tests
  t.comment('Testing appendRole()')
  t.throws(() => correctTx.appendRole(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.appendRole(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.appendRole('', 'new_user_role').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.appendRole('@@@', 'new_user_role').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.appendRole(adminAccountId, '').build(), /Wrongly formed role_id, passed value: ''/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctTx.appendRole(adminAccountId, '@@@').build(), /Wrongly formed role_id, passed value: '@@@'/, 'Should throw Wrongly formed role_id')
  t.doesNotThrow(() => correctTx.appendRole(adminAccountId, 'new_user_role').build(), null, 'Should not throw any exceptions')

  // createAsset() tests
  t.comment('Testing createAsset()')
  t.throws(() => correctTx.createAsset(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAsset(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAsset('', ''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAsset('', 'domain', 2).build(), /Wrongly formed asset_name, passed value: ''/, 'Should throw Wrongly formed asset_name')
  t.throws(() => correctTx.createAsset('$$$', 'domain', 2).build(), /Wrongly formed asset_name, passed value: '\$\$\$'/, 'Should throw Wrongly formed asset_name')
  t.throws(() => correctTx.createAsset('coin', '', 2).build(), /Wrongly formed domain_id, passed value: ''/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createAsset('coin', '$$$', 2).build(), /Wrongly formed domain_id, passed value: '\$\$\$'/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createAsset('coin', 'domain', -10).build(), ERROR_MESSAGES.NUMBER_NOT_IN_RANGE, 'Should throw ...argument 4 of type...')
  // t.throws(() => correctTx.createAsset('coin', 'domain', 1.2).build(), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw ...argument 4 of type...')
  t.doesNotThrow(() => correctTx.createAsset('coin', 'domain', 2).build(), null, 'Should not throw any exceptions')

  // createAccount() tests
  t.comment('Testing createAccount()')
  t.throws(() => correctTx.createAccount(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAccount(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAccount('', ''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAccount('', 'domain', keypair.publicKey()).build(), /Wrongly formed account_name, passed value: ''/, 'Should throw Wrongly formed asset_name')
  t.throws(() => correctTx.createAccount('$$$', 'domain', keypair.publicKey()).build(), /Wrongly formed account_name, passed value: '\$\$\$'/, 'Should throw Wrongly formed asset_name')
  t.throws(() => correctTx.createAccount('admin', '', keypair.publicKey()).build(), /Wrongly formed domain_id, passed value: ''/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createAccount('admin', '$$$', keypair.publicKey()).build(), /Wrongly formed domain_id, passed value: '\$\$\$'/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createAccount('admin', 'domain', 'aaa'), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw ...argument 4 of type...')
  t.doesNotThrow(() => correctTx.createAccount('admin', 'domain', keypair.publicKey()).build(), null, 'Should not throw any exceptions')

  // createDomain() tests
  t.comment('Testing createDomain()')
  t.throws(() => correctTx.createDomain(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createDomain(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createDomain('', 'new_user_role').build(), /Wrongly formed domain_id, passed value: ''/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createDomain('$$$', 'new_user_role').build(), /Wrongly formed domain_id, passed value: '\$\$\$'/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createDomain('domain', '').build(), /Wrongly formed role_id, passed value: ''/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctTx.createDomain('domain', '@@@').build(), /Wrongly formed role_id, passed value: '@@@'/, 'Should throw Wrongly formed role_id')
  t.doesNotThrow(() => correctTx.createDomain('domain', 'new_user_role').build(), null, 'Should not throw any exceptions')

  // createRole() tests
  t.comment('Testing createRole()')
  t.throws(() => correctTx.createRole(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createRole(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')

  let emptyPerm = new iroha.RolePermissionSet()
  let validPermissions = new iroha.RolePermissionSet()
  validPermissions.set(iroha.Role.kAddPeer)
  validPermissions.set(iroha.Role.kAddAssetQty)

  t.throws(() => correctTx.createRole('new_user_role', emptyPerm).build(), /Permission set should contain at least one permission/, 'Should throw Permission set should contain at least one permission')
  t.throws(() => correctTx.createRole('', validPermissions).build(), /Wrongly formed role_id, passed value: ''/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctTx.createRole('@@@', validPermissions).build(), /Wrongly formed role_id, passed value: '@@@'/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctTx.createRole('new_user_role', '').build(), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw WRONG_ARGUMENT_TYPE')
  t.doesNotThrow(() => correctTx.createRole('new_user_role', validPermissions).build(), null, 'Should not throw any exceptions')

  // detachRole() tests
  t.comment('Testing detachRole()')
  t.throws(() => correctTx.detachRole(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.detachRole(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.detachRole('', 'new_user_role').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.detachRole('@@@', 'new_user_role').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.detachRole(adminAccountId, '').build(), /Wrongly formed role_id, passed value: ''/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctTx.detachRole(adminAccountId, '@@@').build(), /Wrongly formed role_id, passed value: '@@@'/, 'Should throw Wrongly formed role_id')
  t.doesNotThrow(() => correctTx.detachRole(adminAccountId, 'new_user_role').build(), null, 'Should not throw any exceptions')

  // grantPermission() tests
  t.comment('Testing grantPermission()')
  t.throws(() => correctTx.grantPermission(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw ILLEGAL_NUMBER_OF_ARGUMENTS')
  t.throws(() => correctTx.grantPermission(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw ILLEGAL_NUMBER_OF_ARGUMENTS')
  t.throws(() => correctTx.grantPermission('', iroha.Grantable.kSetMyQuorum).build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.grantPermission('@@@', iroha.Grantable.kSetMyQuorum).build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.grantPermission(adminAccountId, '').build(), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw WRONG_ARGUMENT_TYPE')
  t.doesNotThrow(() => correctTx.grantPermission(adminAccountId, iroha.Grantable.kSetMyQuorum).build(), null, 'Should not throw any exceptions')

  // revokePermission() tests
  t.comment('Testing revokePermission()')
  t.throws(() => correctTx.revokePermission(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw ILLEGAL_NUMBER_OF_ARGUMENTS')
  t.throws(() => correctTx.revokePermission(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw ILLEGAL_NUMBER_OF_ARGUMENTS')
  t.throws(() => correctTx.revokePermission('', iroha.Grantable.kSetMyQuorum).build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.revokePermission('@@@', iroha.Grantable.kSetMyQuorum).build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.revokePermission(adminAccountId, '').build(), ERROR_MESSAGES.WRONG_ARGUMENT_TYPE, 'Should throw WRONG_ARGUMENT_TYPE')
  t.doesNotThrow(() => correctTx.revokePermission(adminAccountId, iroha.Grantable.kSetMyQuorum).build(), null, 'Should not throw any exceptions')

  // setAccountDetail() tests
  t.comment('Testing setAccountDetail()')
  t.throws(() => correctTx.setAccountDetail(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.setAccountDetail(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.setAccountDetail('', ''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.setAccountDetail('', 'key', 'value').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.setAccountDetail('@@@', 'key', 'value').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.setAccountDetail(adminAccountId, '', 'value').build(), /Wrongly formed key, passed value: ''/, 'Should throw Wrongly formed key')
  t.throws(() => correctTx.setAccountDetail(adminAccountId, '@@@', 'value').build(), /Wrongly formed key, passed value: '@@@'/, 'Should throw Wrongly formed key')
  t.doesNotThrow(() => correctTx.setAccountDetail(adminAccountId, 'key', 'value').build(), null, 'Should not throw any exceptions')
  t.doesNotThrow(() => correctTx.setAccountDetail(adminAccountId, 'key', '大卫王').build(), null, 'Should allow UTF-16 characters')

  // setAccountQuorum() tests
  t.comment('Testing setAccountQuorum()')
  t.throws(() => correctTx.setAccountQuorum(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.setAccountQuorum(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.setAccountQuorum('', 10).build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.setAccountQuorum('@@@', 10).build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.setAccountQuorum(adminAccountId, 'kek').build(), ERROR_MESSAGES.CANNOT_CONVERT_ARG, 'Should throw ...argument 3 of type...')
  t.throws(() => correctTx.setAccountQuorum(adminAccountId, 0).build(), /Quorum should be within range \(0, 128\]/, 'Should throw Quorum should be within range (0, 128]')
  t.throws(() => correctTx.setAccountQuorum(adminAccountId, 200).build(), /Quorum should be within range \(0, 128\]/, 'Should throw Quorum should be within range (0, 128]')
  t.doesNotThrow(() => correctTx.setAccountQuorum(adminAccountId, 10).build(), null, 'Should not throw any exceptions')

  // subtractAssetQuantity() tests
  t.comment('Testing subtractAssetQuantity()')
  t.throws(() => correctTx.subtractAssetQuantity(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.subtractAssetQuantity(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.subtractAssetQuantity('', ''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.subtractAssetQuantity('', '', '').build(), /SubtractAssetQuantity: \[\[Wrongly formed account_id, passed value: ''(.*)Wrongly formed asset_id, passed value: ''(.*)Amount must be greater than 0, passed value: 0 \]\]/, 'Should throw wrongly formed account_id, asset_id, Amount must be greater than 0')
  t.throws(() => correctTx.subtractAssetQuantity(adminAccountId, assetId, '0').build(), /SubtractAssetQuantity: \[\[Amount must be greater than 0, passed value: 0 \]\]/, 'Should throw Amount must be greater than 0')
  // TODO: MAYBE Throw an exception on real amount
  // t.throws(() => correctTx.subtractAssetQuantity(adminAccountId, assetId, '0.123').build(), /SubtractAssetQuantity: \[\[Amount must be integer, passed value: 0.123 \]\]/, 'Should throw Amount must be integer')
  t.throws(() => correctTx.subtractAssetQuantity('', assetId, '1000').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.subtractAssetQuantity('@@@', assetId, '1000').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.subtractAssetQuantity(adminAccountId, '', '1000').build(), /Wrongly formed asset_id, passed value: ''/, 'Should throw Wrongly formed asset_id')
  t.throws(() => correctTx.subtractAssetQuantity(adminAccountId, '###', '1000').build(), /Wrongly formed asset_id, passed value: '###'/, 'Should throw Wrongly formed asset_id')
  t.doesNotThrow(() => correctTx.subtractAssetQuantity(adminAccountId, assetId, '1000').build(), null, 'Should not throw any exceptions')

  // transferAsset() tests
  t.comment('Testing transferAsset()')
  t.throws(() => correctTx.transferAsset(), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.transferAsset(''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.transferAsset('', ''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.transferAsset('', '', ''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.transferAsset('', '', '', ''), ERROR_MESSAGES.ILLEGAL_NUMBER_OF_ARGUMENTS, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.transferAsset('', testAccountId, assetId, 'some message', '100').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctTx.transferAsset('@@@', testAccountId, assetId, 'some message', '100').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctTx.transferAsset(adminAccountId, '', assetId, 'some message', '100').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctTx.transferAsset(adminAccountId, '@@@', assetId, 'some message', '100').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctTx.transferAsset(adminAccountId, testAccountId, '', 'some message', '100').build(), /Wrongly formed asset_id, passed value: ''/, 'Should throw Wrongly formed asset_id,')
  t.throws(() => correctTx.transferAsset(adminAccountId, testAccountId, '@@@', 'some message', '100').build(), /Wrongly formed asset_id, passed value: '@@@'/, 'Should throw Wrongly formed asset_id,')
  t.throws(() => correctTx.transferAsset(adminAccountId, testAccountId, assetId, 'some mesage', '0').build(), /TransferAsset: \[\[Amount must be greater than 0, passed value: 0 \]\]/, 'Should throw Amount must be greater than 0')
  // TODO: MAYBE Throw an exception on real amount
  // t.throws(() => correctTx.transferAsset(adminAccountId, testAccountId, assetId, 'some mesage', '0.123').build(), /TransferAsset: \[\[Amount must be integer, passed value: 0.123 \]\]/, 'Should throw Amount must be integer')
  t.doesNotThrow(() => correctTx.transferAsset(adminAccountId, testAccountId, assetId, 'some mesage', '100').build(), null, 'Should not throw any exceptions')

  t.end()
})
