import iroha
import unittest
import time
import sys

from google.protobuf.message import DecodeError
import block_pb2 as blk

# TODO luckychess 8.08.2018 add test for number of methods
# in interface and proto implementation IR-1080

class BuilderTest(unittest.TestCase):
  def test_empty_tx(self):
    with self.assertRaises(ValueError):
      iroha.ModelTransactionBuilder().build()
  def generate_base(self):
    return iroha.ModelTransactionBuilder().txCounter(123)\
                               .createdTime(int(time.time() * 1000))\
                               .creatorAccountId("admin@test")\

  def setUp(self):
    self.keys = iroha.ModelCrypto().generateKeypair()
    self.builder = self.generate_base()

  def set_add_peer(self):
    self.builder.addPeer("123.123.123.123", self.keys.publicKey())

  def test_outdated_add_peer(self):
    self.set_add_peer()
    for i in [0, int((time.time() - 100000) * 1000), int((time.time() + 1) * 1000)]:
      with self.assertRaises(ValueError):
        self.builder.createdTime(i).build()

  def test_add_peer_with_invalid_creator(self):
    self.set_add_peer()
    for s in ["invalid", "@invalid", "invalid"]:
      with self.assertRaises(ValueError):
        self.builder.creatorAccountId(s).build()

  def test_add_peer_with_invalid_key_size(self):
    for k in ['9' * 13, '9' * (len(self.keys.publicKey().blob()) - 1), '9' *  (len(self.keys.publicKey().blob()) + 1), '']:
      with self.assertRaises(ValueError):
        self.generate_base().addPeer("123.123.123.123", iroha.PublicKey(k)).build()

  def test_add_peer_with_invalid_host(self):
    for k in ["257.257.257.257", "host#host", "asd@asd", 'a' * 257, "ab..cd"]:
      with self.assertRaises(ValueError) as err:
        self.generate_base().addPeer(k, self.keys.publicKey()).build()

  def proto(self, tx):
    return iroha.ModelProtoTransaction().signAndAddSignature(tx, self.keys)

  def check_proto_tx(self, blob):
    try:
      if sys.version_info[0] == 2:
        tmp = ''.join(map(chr, blob.blob()))
      else:
        tmp = bytes(blob.blob())
      blk.Transaction.FromString(tmp)
    except DecodeError as e:
      print(e)
      return False
    return True

  def test_add_peer(self):
    tx = self.builder.addPeer("123.123.123.123:123", self.keys.publicKey()).build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_add_signatory(self):
    tx = self.builder.addSignatory("admin@test", self.keys.publicKey()).build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_add_asset_quantity(self):
    tx = self.builder.addAssetQuantity("admin@test", "asset#domain", "12.345").build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_remove_signatory(self):
    tx = self.builder.removeSignatory("admin@test", self.keys.publicKey()).build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_create_account(self):
    tx = self.builder.createAccount("admin", "domain", self.keys.publicKey()).build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_create_domain(self):
    tx = self.builder.createDomain("domain", "role").build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_set_account_quorum(self):
    tx = self.builder.setAccountQuorum("admin@test", 123).build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_transfer_asset(self):
    tx = self.builder.transferAsset("from@test", "to@test", "asset#test", "description", "123.456").build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_set_account_detail(self):
    tx = self.builder.setAccountDetail("admin@test", "fyodor", "kek").build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

if __name__ == '__main__':
  unittest.main()
