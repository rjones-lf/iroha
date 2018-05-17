#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

import iroha
import unittest
import time
import sys
from google.protobuf.message import DecodeError
import queries_pb2 as qry

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
    self.builder = self.base()

  def set_get_account(self):
    self.builder.getAccount("user@test")

  def base(self):
    return iroha.ModelQueryBuilder().queryCounter(123)\
      .createdTime(int(time.time() * 1000))\
      .creatorAccountId("admin@test")

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

  def test_empty_query(self):
    with self.assertRaises(ValueError):
      iroha.ModelQueryBuilder().build()

  # ====================== GetAccount Tests ======================

  def test_get_account(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        query = self.builder.getAccount("{}@{}".format(name, domain)).build()
        self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_outdated_get_account(self):
    self.set_get_account()
    for i in [0, int((time.time() - 100000) * 1000), int((time.time() + 1) * 1000)]:
      with self.assertRaises(ValueError):
        self.builder.createdTime(i).build()

  def test_get_account_with_invalid_creator(self):
    self.set_get_account()
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.builder.creatorAccountId("{}@test".format(name)).build()

  def test_get_account_with_invalid_creator_domain(self):
    self.set_get_account()
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.builder.creatorAccountId("admin@{}".format(domain)).build()

  def test_get_account_with_empty_creator_domain(self):
    self.set_get_account()
    with self.assertRaises(ValueError):
      self.builder.creatorAccountId("").build()

  def test_get_account_invalid_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().getAccount("{}@test".format(name)).build()

  def test_get_account_invalid_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().getAccount("admin@{}".format(domain)).build()

  def test_get_account_with_empty_name(self):
    with self.assertRaises(ValueError):
      self.base().getAccount("").build()

  # ====================== GetSignatories Tests ======================

  def test_get_signatories(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        query = self.builder.getSignatories("{}@{}".format(name, domain)).build()
        self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_signatories_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().getSignatories("{}@test".format(name)).build()

  def test_get_signatories_invalid_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().getSignatories("admin@{}".format(domain)).build()

  def test_get_signatories_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().getSignatories("").build()

  # ====================== GetAccountTransactions Tests ======================

  def test_get_account_transactions(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        query = self.builder.getAccountTransactions("{}@{}".format(name, domain)).build()
        self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_account_transactions_invalid_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().getAccountTransactions("{}@test".format(name)).build()

  def test_get_account_transactions_invalid_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().getAccountTransactions("admin@{}".format(domain)).build()

  def test_get_account_transactions_with_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().getAccountTransactions("").build()

  # ====================== GetAccountAssetTransactions Tests ======================

  def test_get_account_asset_transactions(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        query = self.builder.getAccountAssetTransactions("{}@{}".format(name, domain), "{}#{}".format(name, domain)).build()
        self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_account_asset_transactions_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().getAccountAssetTransactions("{}@test".format(name), "coin#test").build()

  def test_get_account_asset_transactions_invalid_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().getAccountAssetTransactions("admin@{}".format(domain), "coin#test").build()

  def test_get_account_asset_transactions_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().getAccountAssetTransactions("", "coin#test").build()

  def test_get_account_asset_transactions_invalid_asset_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().getAccountAssetTransactions("admin@test", "{}#test".format(name)).build()

  def test_get_account_asset_transactions_invalid_asset_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().getAccountAssetTransactions("admin@test", "admin#{}".format(domain)).build()

  def test_get_account_asset_transactions_empty_asset(self):
    with self.assertRaises(ValueError):
      self.base().getAccountAssetTransactions("admin@test", "").build()

  # ====================== GetAccountAssets Tests ======================

  def test_get_account_assets(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        query = self.builder.getAccountAssets("{}@{}".format(name, domain), "{}#{}".format(name, domain)).build()
        self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_account_assets_invalid_account(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().getAccountAssets("{}@test".format(name), "coin#test").build()

  def test_get_account_assets_invalid_account_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().getAccountAssets("admin@{}".format(domain), "coin#test").build()

  def test_get_account_assets_empty_account(self):
    with self.assertRaises(ValueError):
      self.base().getAccountAssets("", "coin#test").build()

  def test_get_account_assets_invalid_asset_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().getAccountAssets("admin@test", "{}#test".format(name)).build()

  def test_get_account_assets_invalid_asset_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().getAccountAssets("admin@test", "admin#{}".format(domain)).build()

  def test_get_account_assets_empty_asset(self):
    with self.assertRaises(ValueError):
      self.base().getAccountAssets("admin@test", "").build()

  # ====================== GetRoles Tests ======================

  def test_get_roles(self):
    query = self.builder.getRoles().build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  # ====================== GetAssetInfo Tests ======================

  def test_get_asset_info(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        query = self.builder.getAssetInfo("{}#{}".format(name, domain)).build()
        self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_asset_info_invalid_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().getAssetInfo("{}#test".format(name)).build()

  def test_get_asset_info_invalid_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().getAssetInfo("admin#{}".format(domain)).build()

  def test_get_asset_info_empty_asset_name(self):
    with self.assertRaises(ValueError):
      self.base().getAssetInfo("").build()

  # ====================== GetRolePermissions Tests ======================

  def test_get_role_permissions(self):
    for name in VALID_NAMES_1:
      query = self.builder.getRolePermissions(name).build()
      self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_role_permissions_with_invalid_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().getRolePermissions(name).build()

  # ====================== GetTransactions Tests ======================

  def test_get_transactions(self):
    hv = iroha.HashVector()
    hv.append(iroha.Hash("1" * 32))
    hv.append(iroha.Hash("2" * 32))
    self.assertTrue(hv.size() == 2)

    query = self.builder.getTransactions(hv).build()
    self.assertTrue(self.check_proto_query(self.proto(query)))

  # TODO igor-egorov, 11.05.2018, IR-1322
  @unittest.skip("IR-1322")
  def test_get_transactions_with_empty_vector(self):
    with self.assertRaises(ValueError):
      self.base().getTransactions(iroha.HashVector()).build()

  # TODO igor-egorov, 11.05.2018, IR-1325
  @unittest.skip("IR-1325")
  def test_get_transactions_with_invalid_hash_sizes(self):
    hashes = [
      "",
      "1",
      "1" * 31,
      "1" * 33
    ]
    for h in hashes:
      hv = iroha.HashVector()
      hv.append(iroha.Hash(h))
      with self.assertRaises(ValueError):
        self.base().getTransactions(hv).build()

  # TODO igor-egorov, 11.05.2018, IR-1325
  @unittest.skip("IR-1325")
  def test_get_transactions_with_one_valid_and_one_invalid_hash_1(self):
    hv = iroha.HashVector()
    hv.append(iroha.Hash("1" * 32))
    hv.append(iroha.Hash("1"))

    with self.assertRaises(ValueError):
      self.base().getTransactions(hv).build()

  # TODO igor-egorov, 11.05.2018, IR-1325
  @unittest.skip("IR-1325")
  def test_get_transactions_with_one_valid_and_one_invalid_hash_2(self):
    hv = iroha.HashVector()
    hv.append(iroha.Hash("1"))
    hv.append(iroha.Hash("1" * 32))

    with self.assertRaises(ValueError):
      self.base().getTransactions(hv).build()


  # ====================== GetAccountDetail Tests ======================

  def test_get_account_detail(self):
    for domain in VALID_DOMAINS:
      for name in VALID_NAMES_1:
        query = self.builder.getAccountDetail("{}@{}".format(name, domain)).build()
        self.assertTrue(self.check_proto_query(self.proto(query)))

  def test_get_account_detail_invalid_name(self):
    for name in INVALID_NAMES_1:
      with self.assertRaises(ValueError):
        self.base().getAccountDetail("{}@test".format(name)).build()

  def test_get_account_detail_invalid_domain(self):
    for domain in INVALID_DOMAINS:
      with self.assertRaises(ValueError):
        self.base().getAccountDetail("admin@{}".format(domain)).build()

  def test_get_account_detail_with_empty_name(self):
    with self.assertRaises(ValueError):
      self.base().getAccountDetail("").build()

if __name__ == '__main__':
  unittest.main()
