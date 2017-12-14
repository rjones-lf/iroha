import iroha
import unittest
import time

class BuilderTest(unittest.TestCase):
  def test_empty_tx(self):
    with self.assertRaises(ValueError):
      iroha.ModelBuilder().build()

  def setUp(self):
    self.keys = iroha.ModelCrypto().generateKeypair()
    self.builder = iroha.ModelBuilder().txCounter(123)\
                                       .createdTime(int(time.time() * 1000))\
                                       .creatorAccountId("admin@test")

  def proto_size(self, tx):
    return iroha.ModelTransactionProto().signAndAddSignature(tx, self.keys).size()

  def test_keygen(self):
    self.assertEqual(len(self.keys.privateKey().blob()), 64)
    self.assertEqual(len(self.keys.publicKey().blob()), 32)

  def test_add_peer(self):
    tx = self.builder.addPeer("123.123.123.123", self.keys.publicKey()).build()
    self.assertEqual(self.proto_size(tx), 180)

  def test_add_signatory(self):
    tx = self.builder.addSignatory("admin@test", self.keys.publicKey()).build()
    self.assertEqual(self.proto_size(tx), 175)

  def test_add_asset_quantity(self):
    tx = self.builder.addAssetQuantity("admin@test", "asset#domain", "12.345").build()
    self.assertEqual(self.proto_size(tx), 164)

  def test_remove_signatory(self):
    tx = self.builder.removeSignatory("admin@test", self.keys.publicKey()).build()
    self.assertEqual(self.proto_size(tx), 175)

  def test_create_account(self):
    tx = self.builder.createAccount("admin", "domain", self.keys.publicKey()).build()
    self.assertEqual(self.proto_size(tx), 178)

  def test_create_domain(self):
    tx = self.builder.createDomain("domain", "role").build()
    self.assertEqual(self.proto_size(tx), 143)

  def test_set_account_quorum(self):
    tx = self.builder.setAccountQuorum("admin@test", 123).build()
    self.assertEqual(self.proto_size(tx), 143)

  def test_transfer_asset(self):
    tx = self.builder.transferAsset("from@test", "to@test", "asset#test", "description", "123.456").build()
    self.assertEqual(self.proto_size(tx), 184)

if __name__ == '__main__':
  unittest.main()
