import iroha
import unittest
import time
import sys
from google.protobuf.message import DecodeError
import queries_pb2 as qry

# TODO luckychess 8.08.2018 add test for number of methods
# in interface and proto implementation IR-1080

class BuilderTest(unittest.TestCase):
  def test_empty_query(self):
    with self.assertRaises(ValueError):
      iroha.ModelQueryBuilder().build()

  def generate_base(self):
    return iroha.ModelQueryBuilder().queryCounter(123)\
                               .createdTime(int(time.time() * 1000))\
                               .creatorAccountId("admin@test")\

  def setUp(self):
    self.keys = iroha.ModelCrypto().generateKeypair()
    self.builder = self.generate_base()

  def set_get_account(self):
    self.builder.getAccount("user@test")

  def test_outdated_add_peer(self):
    self.set_get_account()
    for i in [0, int((time.time() - 100000) * 1000), int((time.time() + 1) * 1000)]:
      with self.assertRaises(ValueError):
        self.builder.createdTime(i).build()

  def test_add_peer_with_invalid_creator(self):
    self.set_get_account()
    for s in ["invalid", "@invalid", "invalid"]:
      with self.assertRaises(ValueError):
        self.builder.creatorAccountId(s).build()

  def proto(self, query):
    return iroha.ModelProtoQuery().signAndAddSignature(query, self.keys)

  def check_proto_query(self, blob):
    try:
      if sys.version_info[0] == 2:
        tmp = ''.join(map(chr, blob.blob()))
      else:
        tmp = bytes(blob.blob())
        qry.Query.FromString(tmp)
    except DecodeError as e:
      print(e)
      return False
    return True

  def test_get_account(self):
    query = self.builder.getAccount("user@test").build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_signatories(self):
    query = self.builder.getSignatories("user@test").build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_account_transactions(self):
    query = self.builder.getAccountTransactions("user@test").build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_account_asset_transactions(self):
    query = self.builder.getAccountAssetTransactions("user@test", "coin#test").build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_account_assets(self):
    query = self.builder.getAccountAssets("user@test", "coin#test").build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_roles(self):
    query = self.builder.getRoles().build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_asset_info(self):
    query = self.builder.getAssetInfo("coin#test").build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_role_permissions(self):
    query = self.builder.getRolePermissions("user").build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_transactions(self):
    query = self.builder.getTransactions([iroha.Hash("1" * 32), iroha.Hash("2" * 32)]).build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_account_detail(self):
    query = self.builder.getAccountDetail("user@test", "hello").build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

if __name__ == '__main__':
  unittest.main()
