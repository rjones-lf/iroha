var test = require('tape')
var iroha = require('../index')

const publicKey = '407e57f50ca48969b08ba948171bb2435e035d82cec417e18e4a38f5fb113f83'
const privateKey = '1d7e0a32ee0affeb4d22acd73c2c6fb6bd58e266c8c2ce4fa0ffe3dd6a253ffb'

const adminAccountId = 'admin@test'
const assetId = 'coin#test'
const testAccountId = 'test@test'

test('ModelTransactionBuilder tests', function (t) {
  t.plan(130)

  let crypto = new iroha.ModelCrypto()
  let keypair = crypto.convertFromExisting(publicKey, privateKey)

  let txBuilder = new iroha.ModelTransactionBuilder()
  const time = (new Date()).getTime()
  const address = '0.0.0.0:50051'

  t.comment('Basic TransactionBuilder tests')

  t.throws(() => txBuilder.build(), /Transaction should contain at least one command/, 'Should throw exception 0 commands in transaction, wrong creator_account_id, timestamp')
  t.throws(() => txBuilder.creatorAccountId(adminAccountId).build(), /Transaction should contain at least one command/, 'Should throw exception about zero commands in transaction, wrong timestamp')
  t.throws(() => txBuilder.creatorAccountId(adminAccountId).createdTime(0).build(), /Transaction should contain at least one command bad timestamp: too old/, 'Should throw 0 commands + bad timestamp: too old')
  t.throws(() => txBuilder.creatorAccountId(adminAccountId).createdTime(time).build(), /Transaction should contain at least one command/, 'Should throw 0 commands')
  t.throws(() => txBuilder.creatorAccountId('').createdTime(time).build(), /Transaction should contain at least one command Wrongly formed creator_account_id, passed value: ''/, 'Should throw 0 commands + Wrongly formed creator_account_id')
  t.throws(() => txBuilder.creatorAccountId('@@@').createdTime(time).build(), /Transaction should contain at least one command Wrongly formed creator_account_id, passed value: '@@@'/, 'Should throw 0 commands + Wrongly formed creator_account_id')
  t.throws(() => txBuilder.creatorAccountId(adminAccountId).createdTime(time).build(), /Transaction should contain at least one command/, 'Should throw exception about zero commands in transaction')

  // Transaction with valid creatorAccountId and createdTime
  let correctTx = txBuilder.creatorAccountId(adminAccountId).createdTime(time)

  // addAssetQuantity() tests
  t.comment('Testing addAssetQuantity()')
  t.throws(() => correctTx.addAssetQuantity(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addAssetQuantity(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addAssetQuantity('', ''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addAssetQuantity('', '', '').build(), /AddAssetQuantity: \[\[Wrongly formed account_id, passed value: '' Wrongly formed asset_id, passed value: '' Amount must be greater than 0, passed value: 0 \]\]/, 'Should throw wrongly formed account_id, asset_id, Amount must be greater than 0')
  t.throws(() => correctTx.addAssetQuantity(adminAccountId, assetId, '0').build(), /AddAssetQuantity: \[\[Amount must be greater than 0, passed value: 0 \]\]/, 'Should throw Amount must be greater than 0')
  t.throws(() => correctTx.addAssetQuantity('', assetId, '1000').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.addAssetQuantity('@@@', assetId, '1000').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.addAssetQuantity(adminAccountId, '', '1000').build(), /Wrongly formed asset_id, passed value: ''/, 'Should throw Wrongly formed asset_id')
  t.throws(() => correctTx.addAssetQuantity(adminAccountId, '###', '1000').build(), /Wrongly formed asset_id, passed value: '###'/, 'Should throw Wrongly formed asset_id')
  t.doesNotThrow(() => correctTx.addAssetQuantity(adminAccountId, assetId, '1000').build(), null, 'Should not throw any exceptions')

  // addPeer() tests
  t.comment('Testing addPeer()')
  t.throws(() => correctTx.addPeer(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addPeer(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addPeer('', keypair.publicKey()).build(), /Wrongly formed peer address/, 'Should throw exception about wrongly formed peer address')
  t.throws(() => correctTx.addPeer(address, '').build(), /argument 3 of type 'shared_model::crypto::PublicKey const &'/, 'Should throw ...argument 3 of type...')
  t.doesNotThrow(() => correctTx.addPeer(address, keypair.publicKey()).build(), null, 'Should not throw any exceptions')

  // addSignatory() tests
  t.comment('Testing addSignatory()')
  t.throws(() => correctTx.addSignatory(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addSignatory(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.addSignatory('', keypair.publicKey()).build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.addSignatory('@@@', keypair.publicKey()).build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.addSignatory(adminAccountId, '').build(), /argument 3 of type 'shared_model::crypto::PublicKey const &'/, 'Should throw ...argument 3 of type...')
  t.doesNotThrow(() => correctTx.addSignatory(adminAccountId, keypair.publicKey()).build(), null, 'Should not throw any exceptions')

  // removeSignatory() tests
  t.comment('Testing removeSignatory()')
  t.throws(() => correctTx.removeSignatory(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.removeSignatory(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.removeSignatory('', keypair.publicKey()).build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.removeSignatory('@@@', keypair.publicKey()).build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.removeSignatory(adminAccountId, '').build(), /argument 3 of type 'shared_model::crypto::PublicKey const &'/, 'Should throw ...argument 3 of type...')
  t.doesNotThrow(() => correctTx.removeSignatory(adminAccountId, keypair.publicKey()).build(), null, 'Should not throw any exceptions')

  // appendRole() tests
  t.comment('Testing appendRole()')
  t.throws(() => correctTx.appendRole(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.appendRole(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.appendRole('', 'ruser').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.appendRole('@@@', 'ruser').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.appendRole(adminAccountId, '').build(), /Wrongly formed role_id, passed value: ''/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctTx.appendRole(adminAccountId, '@@@').build(), /Wrongly formed role_id, passed value: '@@@'/, 'Should throw Wrongly formed role_id')
  // TODO: 8 symbols
  t.doesNotThrow(() => correctTx.appendRole(adminAccountId, 'ruser').build(), null, 'Should not throw any exceptions')

  // createAsset() tests
  t.comment('Testing createAsset()')
  t.throws(() => correctTx.createAsset(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAsset(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAsset('', ''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAsset('', 'domain', 2).build(), /Wrongly formed asset_name, passed value: ''/, 'Should throw Wrongly formed asset_name')
  t.throws(() => correctTx.createAsset('$$$', 'domain', 2).build(), /Wrongly formed asset_name, passed value: '\$\$\$'/, 'Should throw Wrongly formed asset_name')
  t.throws(() => correctTx.createAsset('coin', '', 2).build(), /Wrongly formed domain_id, passed value: ''/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createAsset('coin', '$$$', 2).build(), /Wrongly formed domain_id, passed value: '\$\$\$'/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createAsset('coin', 'domain', -10).build(), /argument 4 of type 'shared_model::interface::types::PrecisionType'/, 'Should throw ...argument 4 of type...')
  // t.throws(() => correctTx.createAsset('coin', 'domain', 1.2).build(), /argument 4 of type 'shared_model::interface::types::PrecisionType'/, 'Should throw ...argument 4 of type...')
  t.doesNotThrow(() => correctTx.createAsset('coin', 'domain', 2).build(), null, 'Should not throw any exceptions')

  // createAccount() tests
  t.comment('Testing createAccount()')
  t.throws(() => correctTx.createAccount(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAccount(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAccount('', ''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createAccount('', 'domain', keypair.publicKey()).build(), /Wrongly formed account_name, passed value: ''/, 'Should throw Wrongly formed asset_name')
  t.throws(() => correctTx.createAccount('$$$', 'domain', keypair.publicKey()).build(), /Wrongly formed account_name, passed value: '\$\$\$'/, 'Should throw Wrongly formed asset_name')
  t.throws(() => correctTx.createAccount('admin', '', keypair.publicKey()).build(), /Wrongly formed domain_id, passed value: ''/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createAccount('admin', '$$$', keypair.publicKey()).build(), /Wrongly formed domain_id, passed value: '\$\$\$'/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createAccount('admin', 'domain', 'aaa'), /argument 4 of type 'shared_model::crypto::PublicKey/, 'Should throw ...argument 4 of type...')
  t.doesNotThrow(() => correctTx.createAccount('admin', 'domain', keypair.publicKey()).build(), null, 'Should not throw any exceptions')

  // createDomain() tests
  t.comment('Testing createDomain()')
  t.throws(() => correctTx.createDomain(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createDomain(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createDomain('', 'ruser').build(), /Wrongly formed domain_id, passed value: ''/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createDomain('$$$', 'ruser').build(), /Wrongly formed domain_id, passed value: '\$\$\$'/, 'Should throw Wrongly formed domain_id')
  t.throws(() => correctTx.createDomain('domain', '').build(), /Wrongly formed role_id, passed value: ''/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctTx.createDomain('domain', '@@@').build(), /Wrongly formed role_id, passed value: '@@@'/, 'Should throw Wrongly formed role_id')
  t.doesNotThrow(() => correctTx.createDomain('domain', 'ruser').build(), null, 'Should not throw any exceptions')

  // createRole() tests
  t.comment('Testing createRole()')
  t.throws(() => correctTx.createRole(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.createRole(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  let sv = new iroha.StringVector()
  sv.add('permission1')
  sv.add('permission2')
  t.throws(() => correctTx.createRole('', sv).build(), /Wrongly formed role_id, passed value: ''/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctTx.createRole('@@@', sv).build(), /Wrongly formed role_id, passed value: '@@@'/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctTx.createRole('ruser', '').build(), /argument 3 of type 'std::vector< shared_model::interface::types::PermissionNameType >/, 'Should throw ...argument 3 of type...')
  t.doesNotThrow(() => correctTx.createRole('ruser', sv).build(), null, 'Should not throw any exceptions')

  // detachRole() tests
  t.comment('Testing detachRole()')
  t.throws(() => correctTx.detachRole(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.detachRole(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.detachRole('', 'ruser').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.detachRole('@@@', 'ruser').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.detachRole(adminAccountId, '').build(), /Wrongly formed role_id, passed value: ''/, 'Should throw Wrongly formed role_id')
  t.throws(() => correctTx.detachRole(adminAccountId, '@@@').build(), /Wrongly formed role_id, passed value: '@@@'/, 'Should throw Wrongly formed role_id')
  // TODO: 8 symbols
  t.doesNotThrow(() => correctTx.detachRole(adminAccountId, 'ruser').build(), null, 'Should not throw any exceptions')

  // grantPermission() tests
  t.comment('Testing grantPermission()')
  t.throws(() => correctTx.grantPermission(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.grantPermission(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.grantPermission('', 'can_read_assets').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.grantPermission('@@@', 'can_read_assets').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.grantPermission(adminAccountId, '').build(), /Wrongly formed permission, passed value: ''/, 'Should throw Wrongly formed permission')
  t.throws(() => correctTx.grantPermission(adminAccountId, '@@@').build(), /Wrongly formed permission, passed value: '@@@'/, 'Should throw Wrongly formed permission')
  t.doesNotThrow(() => correctTx.grantPermission(adminAccountId, 'can_read_assets').build(), null, 'Should not throw any exceptions')

  // revokePermission() tests
  t.comment('Testing revokePermission()')
  t.throws(() => correctTx.revokePermission(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.revokePermission(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.revokePermission('', 'can_read_assets').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.revokePermission('@@@', 'can_read_assets').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.revokePermission(adminAccountId, '').build(), /Wrongly formed permission, passed value: ''/, 'Should throw Wrongly formed permission')
  t.throws(() => correctTx.revokePermission(adminAccountId, '@@@').build(), /Wrongly formed permission, passed value: '@@@'/, 'Should throw Wrongly formed permission')
  t.doesNotThrow(() => correctTx.revokePermission(adminAccountId, 'can_read_assets').build(), null, 'Should not throw any exceptions')

  // setAccountDetail() tests
  t.comment('Testing setAccountDetail()')
  t.throws(() => correctTx.setAccountDetail(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.setAccountDetail(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.setAccountDetail('', ''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.setAccountDetail('', 'key', 'value').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.setAccountDetail('@@@', 'key', 'value').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.setAccountDetail(adminAccountId, '', 'value').build(), /Wrongly formed key, passed value: ''/, 'Should throw Wrongly formed key')
  t.throws(() => correctTx.setAccountDetail(adminAccountId, '@@@', 'value').build(), /Wrongly formed key, passed value: '@@@'/, 'Should throw Wrongly formed key')
  t.doesNotThrow(() => correctTx.setAccountDetail(adminAccountId, 'key', 'value').build(), null, 'Should not throw any exceptions')

  // setAccountQuorum() tests
  t.comment('Testing setAccountQuorum()')
  t.throws(() => correctTx.setAccountQuorum(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.setAccountQuorum(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.setAccountQuorum('', 10).build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.setAccountQuorum('@@@', 10).build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id')
  t.throws(() => correctTx.setAccountQuorum(adminAccountId, 'kek').build(), /argument 3 of type 'shared_model::interface::types::QuorumType'/, 'Should throw ...argument 3 of type...')
  t.doesNotThrow(() => correctTx.setAccountQuorum(adminAccountId, 10).build(), null, 'Should not throw any exceptions')

  // subtractAssetQuantity() tests
  t.comment('Testing subtractAssetQuantity()')
  t.throws(() => correctTx.subtractAssetQuantity(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.subtractAssetQuantity(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.subtractAssetQuantity('', ''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.subtractAssetQuantity('', '', '').build(), /SubtractAssetQuantity: \[\[Wrongly formed account_id, passed value: '' Wrongly formed asset_id, passed value: '' Amount must be greater than 0, passed value: 0 \]\]/, 'Should throw wrongly formed account_id, asset_id, Amount must be greater than 0')
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
  t.throws(() => correctTx.transferAsset(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.transferAsset(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.transferAsset('', ''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.transferAsset('', '', ''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctTx.transferAsset('', '', '', ''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
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
