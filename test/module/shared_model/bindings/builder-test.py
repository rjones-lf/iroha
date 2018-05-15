#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

import iroha
import unittest
import time
import sys

from google.protobuf.message import DecodeError
import block_pb2 as blk

# TODO luckychess 8.08.2018 add test for number of methods
# in interface and proto implementation IR-1080

# Symbols of type 1 (format [a-z_0-9]{1,32}) are used
# as account_name, asset_name and role_id.
VALID_NAMES_1 = [
  "a",
  "asset",
  "234234",
  "_",
  "_123",
  "123_23",
  "234asset_",
  "__",
  "12345678901234567890123456789012"
]

INVALID_NAMES_1 = [
  "",
  "A",
  "assetV",
  "asSet",
  "asset%",
  "^123",
  "verylongassetname_thenameislonger",
  "verylongassetname_thenameislongerthanitshouldbe",
  "assset-01"
]

# Symbols of type 2 (format [A-Za-z0-9_]{1,64})
# are used as key identifier for setAccountDetail command
VALID_NAMES_2 = [
  "a",
  "A",
  "1",
  "_",
  "Key",
  "Key0_",
  "verylongAndValidKeyName___1110100010___veryveryveryverylongvalid"
]

INVALID_NAMES_2 = [
  "",
  "Key&",
  "key-30",
  "verylongAndValidKeyName___1110100010___veryveryveryverylongvalid1",
  "@@@"
]

VALID_DOMAINS = [
  "test",
  "u9EEA432F",
  "a-hyphen",
  "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad",
  "endWith0",
  "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
  "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
  "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
  "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad"
]

INVALID_DOMAINS = [
  "",
  " ",
  "   ",
  "9start.with.digit",
  "-startWithDash",
  "@.is.not.allowed",
  "no space is allowed",
  "endWith-",
  "label.endedWith-.is.not.allowed",
  "aLabelMustNotExceeds63charactersALabelMustNotExceeds63characters",
  "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
  "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
  "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad." +
  "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPadP",
  "257.257.257.257",
  "domain#domain",
  "asd@asd",
  "ab..cd"
]

INVALID_KEYS = [
  "",
  "a",
  "1" * 31,
  "1" * 33
]

class BuilderTest(unittest.TestCase):

  def setUp(self):
    self.keys = iroha.ModelCrypto().generateKeypair()
    self.pub_key = self.keys.publicKey()
    self.builder = self.base()

  def base(self):
    return iroha.ModelTransactionBuilder()\
      .createdTime(int(time.time() * 1000))\
      .creatorAccountId("admin@test")

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

  def set_add_peer(self):
    self.builder.addPeer("123.123.123.123", self.keys.publicKey())

  def test_empty_tx(self):
    with self.assertRaises(ValueError):
      iroha.ModelTransactionBuilder().build()

  # ====================== AddPeer Tests ======================

  def test_add_peer(self):
    tx = self.builder.addPeer("123.123.123.123:123", self.keys.publicKey()).build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_add_peer_valid_domains(self):
    for domain in VALID_DOMAINS:
      tx = self.builder.addPeer("{}:123".format(domain), self.keys.publicKey()).build()
      self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_outdated_add_peer(self):
    self.set_add_peer()
    for i in [0, int((time.time() - 100000) * 1000), int((time.time() + 1) * 1000)]:
      with self.assertRaises(ValueError):
        self.builder.createdTime(i).build()

  def test_add_peer_with_invalid_creator(self):
    self.set_add_peer()
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.builder.creatorAccountId("{}@test".format(name)).build()

  def test_add_peer_with_invalid_creator_domain(self):
    self.set_add_peer()
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.builder.creatorAccountId("admin@{}".format(domain)).build()

  def test_add_peer_with_empty_creator(self):
    self.set_add_peer()
    with self.assertRaises(ValueError):
      self.builder.creatorAccountId("").build()

  def test_add_peer_with_invalid_key_size(self):
    for k in INVALID_KEYS:
      with self.assertRaises(ValueError):
        self.base().addPeer("123.123.123.123", iroha.PublicKey(k)).build()

  def test_add_peer_with_invalid_domain(self):
    for k in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().addPeer(k, self.keys.publicKey()).build()

  # ====================== AddSignatory Tests ======================

  def test_add_signatory(self):
    for name in VALID_NAMES_1:
      for domain in VALID_DOMAINS:
        tx = self.builder.addSignatory("{}@{}".format(name, domain), self.keys.publicKey()).build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_add_signatory_invalid_account_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().addSignatory("{}@test".format(name), self.keys.publicKey()).build()

  def test_add_signatory_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().addSignatory("", self.keys.publicKey()).build()

  def test_add_signatory_invalid_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().addSignatory("admin@{}".format(domain), self.keys.publicKey()).build()

  def test_add_signatory_invalid_key(self):
    for key in INVALID_KEYS:
      with self.assertRaises(ValueError):
        self.base().addSignatory("admin@test", iroha.PublicKey(key)).build()

  # ====================== AddAssetQuantity Tests ======================

  def test_add_asset_quantity(self):
    tx = self.builder.addAssetQuantity("admin@test", "asset#domain", "12.345").build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_add_asset_quantity_valid_account_and_asset(self):
    for name in VALID_NAMES_1:
      for domain in VALID_DOMAINS:
        tx = self.builder.addAssetQuantity("{}@{}".format(name, domain), "{}#{}".format(name, domain), "100").build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_add_asset_quantity_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().addAssetQuantity("{}@test".format(name), "coin#test", "10").build()

  def test_add_asset_quantity_invalid_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().addAssetQuantity("admin@{}".format(domain), "coin#test", "10").build()

  def test_add_asset_quantity_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().addAssetQuantity("", "coin#test", "10").build()

  def test_add_asset_quantity_invalid_asset(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().addAssetQuantity("admin@test", "{}#test".format(name), "10").build()

  def test_add_asset_quantity_invalid_asset_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().addAssetQuantity("admin@test", "coin#{}".format(domain), "10").build()

  def test_add_asset_quantity_empty_asset(self):
    with self.assertRaises(ValueError):
      self.base().addAssetQuantity("admin@test", "", "10").build()

  def test_add_asset_quantity_invalid_amount(self):
    for amount in ["", "-12", "-13.45", "chars", "chars10"]:
      with self.assertRaises(ValueError):
        self.base().addAssetQuantity("admin@test", "coin#test", amount).build()

  # ====================== RemoveSignatory Tests ======================

  def test_remove_signatory(self):
    for name in VALID_NAMES_1:
      for domain in VALID_DOMAINS:
        tx = self.builder.removeSignatory("{}@{}".format(name, domain), self.keys.publicKey()).build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_remove_signatory_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().removeSignatory("", self.keys.publicKey()).build()

  def test_remove_signatory_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().removeSignatory("{}@test".format(name), self.keys.publicKey()).build()

  def test_remove_signatory_invalid_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().removeSignatory("admin@{}".format(domain), self.keys.publicKey()).build()

  def test_remove_signatory_invalid_key(self):
    for key in INVALID_KEYS:
      with self.assertRaises(ValueError):
        self.base().removeSignatory("admin@test", iroha.PublicKey(key)).build()

  # ====================== CreateAccount Tests ======================

  def test_create_account(self):
    for name in VALID_NAMES_1:
      for domain in VALID_DOMAINS:
        tx = self.builder.createAccount(name, domain, self.keys.publicKey()).build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_create_account_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().createAccount(name, "domain", self.keys.publicKey()).build()

  def test_create_account_invalid_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().createAccount("admin", domain, self.keys.publicKey()).build()

  def test_create_account_invalid_key(self):
    for key in INVALID_KEYS:
      with self.assertRaises(ValueError):
        self.base().createAccount("admin", "test", iroha.PublicKey(key)).build()

  # ====================== CreateDomain Tests ======================

  def test_create_domain(self):
    for domain in VALID_DOMAINS:
      for role in VALID_NAMES_1:
        tx = self.builder.createDomain(domain, role).build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_create_domain_with_invalid_name(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().createDomain(domain, "role").build()

  def test_create_domain_invalid_role(self):
    for role in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().createDomain("test", role).build()

  # ====================== SetAccountQuorum Tests ======================

  def test_set_account_quorum(self):
    for name in VALID_NAMES_1:
      for domain in VALID_DOMAINS:
        tx = self.builder.setAccountQuorum("{}@{}".format(name, domain), 128).build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_set_account_quorum_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().setAccountQuorum("{}@test".format(name), 123).build()

  def test_set_account_quorum_invalid_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().setAccountQuorum("admin@{}".format(domain), 123).build()

  def test_set_account_quorum_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().setAccountQuorum("", 123).build()

  def test_set_account_quorum_invalid_quantity(self):
    with self.assertRaises(OverflowError):
      self.base().setAccountQuorum("admin@test", -100).build()

    for amount in [0, 129]:
      with self.assertRaises(ValueError):
        self.base().setAccountQuorum("admin@test", amount).build()

  # ====================== TransferAsset Tests ======================

  def test_transfer_asset(self):
    for domain in VALID_DOMAINS:
      for i in range(0, len(VALID_NAMES_1)):
        from_acc = "{}@{}".format(VALID_NAMES_1[i], domain)
        to = "{}@{}".format(VALID_NAMES_1[(i + 1) % len(VALID_NAMES_1)], domain)
        asset = "{}#{}".format(VALID_NAMES_1[(i + 2) % len(VALID_NAMES_1)], domain)
        tx = self.builder.transferAsset(from_acc, to, asset, "description", "123.456").build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_transfer_asset_with_invalid_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().transferAsset("from@test", "to@test", "{}#test".format(name), "description", "100").build()

  def test_transfer_asset_with_invalid_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().transferAsset("from@test", "to@test", "coin#{}".format(domain), "description", "100").build()

  def test_transfer_asset_with_empty_name(self):
    with self.assertRaises(ValueError):
      self.base().transferAsset("from@test", "to@test", "", "description", "100").build()

  def test_transfer_asset_invalid_from_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().transferAsset("{}@test".format(name), "to@test", "coin#test", "description", "100").build()

  def test_transfer_asset_invalid_from_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().transferAsset("from@{}".format(domain), "to@test", "coin#test", "description", "100").build()

  def test_transfer_asset_empty_from_account(self):
    with self.assertRaises(ValueError):
      self.base().transferAsset("", "to@test", "coin#test", "description", "100").build()

  def test_transfer_asset_invalid_to_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().transferAsset("from@test", "{}@test".format(name), "coin#test", "description", "100").build()

  def test_transfer_asset_invalid_to_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().transferAsset("from@test", "to@{}".format(domain), "coin#test", "description", "1").build()

  def test_transfer_asset_empty_to_account(self):
    with self.assertRaises(ValueError):
      self.base().transferAsset("from@test", "", "coin#test", "description", "1").build()

  def test_transfer_asset_description_valid_values(self):
    for descr in ["", "a" * 64]:
      tx = self.builder.transferAsset("from@test", "to@test", "coin#test", descr, "1").build()
      self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_transfer_asset_invalid_description(self):
    descr = "a" * 65
    with self.assertRaises(ValueError):
      self.base().transferAsset("from@test", "to@test", "coin#test", descr, "1").build()

  def test_transfer_asset_maximum_amount(self):
    max_uint_256 = str(2 ** 256 - 1)
    max_uint_256_2 = max_uint_256[:10] + '.' + max_uint_256[10:]
    oversized = str(2 ** 256)

    for amount in [max_uint_256, max_uint_256_2]:
      tx = self.builder.transferAsset("from@test", "to@test", "coin#test", "descr", amount).build()
      self.assertTrue(self.check_proto_tx(self.proto(tx)))

    with self.assertRaises(ValueError):
      self.base().transferAsset("from@test", "to@test", "coin#test", "descr", oversized).build()

  # ====================== SetAccountDetail Tests ======================

  def test_set_account_detail(self):
    for name in VALID_NAMES_1:
      for domain in VALID_DOMAINS:
        tx = self.builder.setAccountDetail("{}@{}".format(name, domain), "fyodor", "kek").build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_set_account_detail_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().setAccountDetail("{}@test".format(name), "fyodor", "true").build()

  def test_set_account_detail_invalid_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().setAccountDetail("admin@{}".format(domain), "fyodor", "true").build()

  def test_set_account_detail_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().setAccountDetail("", "fyodor", "true").build()

  def test_set_account_detail_valid_key(self):
    for key in VALID_NAMES_2:
      tx = self.builder.setAccountDetail("admin@test", key, "true").build()
      self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_set_account_detail_invalid_key(self):
    for key in INVALID_NAMES_2:
      with self.assertRaises(ValueError):
        self.base().setAccountDetail("admin@test", key, "true").build()

  def test_set_account_detail_valid_value(self):
    length = 4 * 1024 * 1024
    value = "a" * length

    for v in ["", value]:
      tx = self.builder.setAccountDetail("admin@test", "fyodor", v).build()
      self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_set_account_detail_oversized_value(self):
    length = 4 * 1024 * 1024 + 1
    value = "a" * length

    with self.assertRaises(ValueError):
      self.base().setAccountDetail("admin@test", "fyodor", value).build()

  # ====================== AppendRole Tests ======================

  def test_append_role(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        tx = self.builder.appendRole("{}@{}".format(name, domain), name).build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_append_role_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().appendRole("{}@test".format(name), "user").build()

  def test_append_role_invalid_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().appendRole("admin@{}".format(domain), "user").build()

  def test_append_role_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().appendRole("", "user").build()

  def test_append_role_with_invalid_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().appendRole("admin@test", name).build()

  # ====================== CreateAsset Tests ======================

  def test_create_asset(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        tx = self.builder.createAsset(name, domain, 6).build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_create_asset_invalid_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().createAsset(name, "test", 6).build()

  def test_create_asset_invalid_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().createAsset("asset", domain, 6).build()

  def test_create_asset_zero_precision(self):
    tx = self.builder.createAsset("asset", "test", 0).build()
    self.assertTrue(self.check_proto_tx(self.proto(tx)))

  # ====================== CreateRole Tests ======================

  def test_create_role(self):
    permissions = iroha.StringVector()
    permissions.append("can_receive")
    permissions.append("can_get_roles")
    self.assertTrue(permissions.size() == 2)

    for name in VALID_NAMES_1:
      tx = self.builder.createRole(name, permissions).build()
      self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_create_role_with_invalid_name(self):
    permissions = iroha.StringVector()
    permissions.append("can_receive")
    permissions.append("can_get_roles")

    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().createRole(name, permissions).build()

  def test_create_role_with_empty_permissions(self):
    permissions = iroha.StringVector()
    self.assertTrue(permissions.size() == 0)

    with self.assertRaises(ValueError):
      self.base().createRole("user", permissions).build()

  # TODO igor-egorov, 11.05.2018, IR-1267
  @unittest.skip("Disabled till IR-1267 will be fixed")
  def test_create_role_wrong_permissions(self):
    permissions = iroha.StringVector()
    permissions.append("wrong_permission")
    permissions.append("can_receive")

    with self.assertRaises(ValueError):
      self.base().createRole("user", permissions).build()

  # TODO igor-egorov, 11.05.2018, IR-1267
  @unittest.skip("Disabled till IR-1267 will be fixed")
  def test_create_role_wrong_permissions_2(self):
    permissions = iroha.StringVector()
    permissions.append("can_receive")
    permissions.append("wrong_permission")

    with self.assertRaises(ValueError):
      self.base().createRole("user", permissions).build()

  # ====================== DetachRole Tests ======================

  def test_detach_role(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        tx = self.builder.detachRole("{}@{}".format(name, domain), "role").build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_detach_role_with_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().detachRole("{}@test".format(name), "role").build()

  def test_detach_role_with_invalid_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().detachRole("admin@{}".format(domain), "role").build()

  def test_detach_role_with_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().detachRole("", "role").build()

  def test_detach_role_with_invalid_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().detachRole("admin@test", name).build()

  # ====================== GrantPermission Tests ======================

  def test_grant_permission(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        tx = self.builder.grantPermission("{}@{}".format(name, domain), "can_set_my_quorum").build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_grant_permission_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().grantPermission("", "can_set_my_quorum").build()

  def test_grant_permission_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().grantPermission("{}@test".format(name), "can_set_my_quorum").build()

  def test_grant_permission_invalid_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().grantPermission("admin@{}".format(domain), "can_set_my_quorum").build()

  def test_grant_permission_with_invalid_name(self):
    permissions = [
      "",
      "random",
      "can_read_assets"  # non-grantable permission
    ]

    for perm in permissions:
      with self.assertRaises(ValueError):
        self.base().grantPermission("admin@test", perm).build()

  # ====================== RevokePermission Tests ======================

  def test_revoke_permission(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        tx = self.builder.revokePermission("{}@{}".format(name, domain), "can_set_my_quorum").build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_revoke_permission_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().revokePermission("{}@test".format(name), "can_set_my_quorum").build()

  def test_revoke_permission_invalid_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().revokePermission("admin@{}".format(domain), "can_set_my_quorum").build()

  def test_revoke_permission_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().revokePermission("", "can_set_my_quorum").build()

  def test_revoke_permission_with_invalid_name(self):
    permissions = [
      "",
      "random",
      "can_read_assets"  # non-grantable permission
    ]

    for perm in permissions:
      with self.assertRaises(ValueError):
        self.base().revokePermission("admin@test", perm).build()

  # ====================== SubtractAssetQuantity Tests ======================

  def test_subtract_asset_quantity(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        tx = self.builder.subtractAssetQuantity("{}@{}".format(name, domain), "{}#{}".format(name, domain), "10").build()
        self.assertTrue(self.check_proto_tx(self.proto(tx)))

  def test_subtract_asset_quantity_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().subtractAssetQuantity("{}@test".format(name), "coin#test", "10").build()

  def test_subtract_asset_quantity_invalid_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().subtractAssetQuantity("admin@{}".format(domain), "coin#test", "10").build()

  def test_subtract_asset_quantity_with_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().subtractAssetQuantity("", "coin#test", "10").build()

  def test_subtract_asset_quantity_invalid_asset_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().subtractAssetQuantity("admin@test", "{}#test".format(name), "10").build()

  def test_subtract_asset_quantity_invalid_asset_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().subtractAssetQuantity("admin@test", "coin#{}".format(domain), "10").build()

  def test_subtract_asset_quantity_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().subtractAssetQuantity("admin@test", "", "10").build()

  def test_subtract_asset_quantity_invalid_amount(self):
    amounts = ["", "0", "chars", "-10", "10chars", "10.10.10"]
    for amount in amounts:
      with self.assertRaises(ValueError):
        self.base().subtractAssetQuantity("admin@test", "coin#test", amount).build()

if __name__ == '__main__':
  unittest.main()
