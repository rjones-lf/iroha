var test = require('tape')
var iroha = require('../index')

const accountId = 'admin@test'
const assetId = 'coin#test'

test('ModelQueryBuilder tests', function (t) {
  t.plan(48)

  let queryBuilder = new iroha.ModelQueryBuilder()
  const time = (new Date()).getTime()

  // Tests for concrete query
  t.comment('Basic QueryBuilder tests')
  t.throws(() => queryBuilder.build(), /Missing concrete query/, 'Should throw Missing concrete query')
  t.throws(() => queryBuilder.creatorAccountId(accountId).build(), /Missing concrete query/, 'Should throw Missing concrete query')
  t.throws(() => queryBuilder.creatorAccountId(accountId).createdTime(time).build(), /Missing concrete query/, 'Should throw Missing concrete query')
  t.throws(() => queryBuilder.creatorAccountId(accountId).createdTime(time).queryCounter(1).build(), /Missing concrete query/, 'Should throw Missing concrete query')
  t.throws(() => queryBuilder.creatorAccountId('').createdTime(time).queryCounter(1).getAccount(accountId).build(), /Wrongly formed creator_account_id, passed value: ''/, 'Should throw Wrongly formed creator_account_id')
  t.throws(() => queryBuilder.creatorAccountId(accountId).createdTime(0).queryCounter(1).getAccount(accountId).build(), /bad timestamp: too old, timestamp: 0, now:/, 'Should throw bad timestamp: too old')
  t.throws(() => queryBuilder.creatorAccountId(accountId).createdTime(time).queryCounter(0).getAccount(accountId).build(), /Counter should be > 0, passed value: 0/, 'Should throw Counter should be > 0')

  // Query with valid queryCounter, creatorAccountId and createdTime
  let correctQuery = queryBuilder.creatorAccountId(accountId).createdTime(time).queryCounter(1)

  // getAccount() tests
  t.comment('Testing getAccount()')
  t.throws(() => correctQuery.getAccount(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccount('').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctQuery.getAccount('@@@').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id,')
  t.doesNotThrow(() => correctQuery.getAccount(accountId).build(), null, 'Should not throw any exceptions')

  // getSignatories() tests
  t.comment('Testing getSignatories()')
  t.throws(() => correctQuery.getSignatories(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getSignatories('').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctQuery.getSignatories('@@@').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id,')
  t.doesNotThrow(() => correctQuery.getSignatories(accountId).build(), null, 'Should not throw any exceptions')

  // getAccountTransactions() tests
  t.comment('Testing getAccountTransactions()')
  t.throws(() => correctQuery.getAccountTransactions(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccountTransactions('').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctQuery.getAccountTransactions('@@@').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id,')
  t.doesNotThrow(() => correctQuery.getAccountTransactions(accountId).build(), null, 'Should not throw any exceptions')

  // getAccountAssetTransactions() tests
  t.comment('Testing getAccountAssetTransactions()')
  t.throws(() => correctQuery.getAccountAssetTransactions(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccountAssetTransactions(''), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccountAssetTransactions('', assetId).build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctQuery.getAccountAssetTransactions('@@@', assetId).build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctQuery.getAccountAssetTransactions(accountId, '').build(), /Wrongly formed asset_id, passed value: ''/, 'Should throw Wrongly formed asset_id,')
  t.throws(() => correctQuery.getAccountAssetTransactions(accountId, '@@@').build(), /Wrongly formed asset_id, passed value: '@@@'/, 'Should throw Wrongly formed asset_id,')
  t.doesNotThrow(() => correctQuery.getAccountAssetTransactions(accountId, assetId).build(), null, 'Should not throw any exceptions')

  // getAccountAssets() tests
  t.comment('Testing getAccountAssets()')
  t.throws(() => correctQuery.getAccountAssets(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccountAssets('').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctQuery.getAccountAssets('@@@').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id,')
  t.doesNotThrow(() => correctQuery.getAccountAssets(accountId).build(), null, 'Should not throw any exceptions')

  // getRoles() tests
  t.comment('Testing getRoles()')
  t.doesNotThrow(() => correctQuery.getRoles().build(), null, 'Should not throw any exceptions')

  // getAssetInfo() tests
  t.comment('Testing getAssetInfo()')
  t.throws(() => correctQuery.getAssetInfo(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAssetInfo('').build(), /Wrongly formed asset_id, passed value: ''/, 'Should throw Wrongly formed asset_id,')
  t.throws(() => correctQuery.getAssetInfo('@@@').build(), /Wrongly formed asset_id, passed value: '@@@'/, 'Should throw Wrongly formed asset_id,')
  t.doesNotThrow(() => correctQuery.getAssetInfo(assetId).build(), null, 'Should not throw any exceptions')

  // getRolePermissions() tests
  t.comment('Testing getRolePermissions()')
  t.throws(() => correctQuery.getRolePermissions(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getRolePermissions('').build(), /Wrongly formed role_id, passed value: ''/, 'Should throw Wrongly formed role_id,')
  t.throws(() => correctQuery.getRolePermissions('@@@').build(), /Wrongly formed role_id, passed value: '@@@'/, 'Should throw Wrongly formed role_id,')
  t.doesNotThrow(() => correctQuery.getRolePermissions('role').build(), null, 'Should not throw any exceptions')

  // getTransactions() tests
  t.comment('Testing getTransactions()')
  t.throws(() => correctQuery.getTransactions(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getTransactions(''), /argument 2 of type 'std::vector< shared_model::crypto::Hash >/, 'Should throw ...argument 2 of type...')

  let hv = new iroha.HashVector()
  hv.add(new iroha.Hash('11111111111111111111111111111111'))
  hv.add(new iroha.Hash('22222222222222222222222222222222'))
  let invalidHv = new iroha.HashVector()
  invalidHv.add(new iroha.Hash(''))
  let emptyHv = new iroha.HashVector()

  t.throws(() => correctQuery.getTransactions(emptyHv).build(), /tx_hashes cannot be empty/, 'Should throw tx_hashes cannot be empty')
  t.throws(() => correctQuery.getTransactions(invalidHv).build(), /Hash has invalid size: 0/, 'Should throw Hash has invalid size')
  t.doesNotThrow(() => correctQuery.getTransactions(hv).build(), null, 'Should not throw any exceptions')

  // getAccountDetail() tests
  t.comment('Testing getAccountDetail()')
  t.throws(() => correctQuery.getAccountDetail(), /Error: Illegal number of arguments/, 'Should throw Illegal number of arguments')
  t.throws(() => correctQuery.getAccountDetail('').build(), /Wrongly formed account_id, passed value: ''/, 'Should throw Wrongly formed account_id,')
  t.throws(() => correctQuery.getAccountDetail('@@@').build(), /Wrongly formed account_id, passed value: '@@@'/, 'Should throw Wrongly formed account_id,')
  t.doesNotThrow(() => correctQuery.getAccountDetail(accountId).build(), null, 'Should not throw any exceptions')

  t.end()
})
