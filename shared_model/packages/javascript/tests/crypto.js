var test = require('tape')
var iroha = require('../index')

const publicKey = '407e57f50ca48969b08ba948171bb2435e035d82cec417e18e4a38f5fb113f83'
const privateKey = '1d7e0a32ee0affeb4d22acd73c2c6fb6bd58e266c8c2ce4fa0ffe3dd6a253ffb'
const randomKey = 'aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa'
const incorrectInputLength = 'aaaaaaaaaaaaaaaa'

test('ModelCrypto tests', function (t) {
  t.plan(8)

  let crypto = new iroha.ModelCrypto()

  t.throws(() => crypto.convertFromExisting(randomKey, randomKey), /Provided keypair is not correct/, 'Should throw "Provided keypair is not correct"')
  t.throws(() => crypto.convertFromExisting(incorrectInputLength, incorrectInputLength), /input string has incorrect length/, 'Should throw "input string has incorrect length"')

  let keypair = crypto.convertFromExisting(publicKey, privateKey)
  t.equal(keypair.publicKey().hex(), publicKey, 'Should be the same as public key was passed to convertFromExisting')
  t.equal(keypair.privateKey().hex(), privateKey, 'Should be the same as private key was passed to convertFromExisting')

  let newKeypair = crypto.generateKeypair()
  t.equal(newKeypair.publicKey().hex().length, publicKey.length, 'Size of generated public key should be the same as size of predefined public key')
  t.equal(newKeypair.privateKey().hex().length, privateKey.length, 'Size of generated private key should be the same as size of predefined private key')

  t.throws(() => crypto.fromPrivateKey(incorrectInputLength), /input string has incorrect length/, 'Should throw "input string has incorrect length"')
  t.equal(crypto.fromPrivateKey(privateKey).publicKey().hex(), publicKey)

  t.end()
})
