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

class BuilderTest(unittest.TestCase):

    def setUp(self):
        self.keys = iroha.ModelCrypto().generateKeypair()
        self.builder = self.base()

    def base(self):
        return iroha.ModelBlocksQueryBuilder().queryCounter(123) \
            .createdTime(int(time.time() * 1000)) \
            .creatorAccountId("admin@test")

    def proto(self, query):
        return iroha.ModelProtoBlocksQuery(query).signAndAddSignature(self.keys).finish()

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

    # ====================== GetAccountDetail Tests ======================

    def test_creator_account_id(self):
        for domain in VALID_DOMAINS:
            for name in VALID_NAMES_1:
                query = self.builder.creatorAccountId("{}@{}".format(name, domain)).build()
                self.assertTrue(self.check_proto_query(self.proto(query)))

    def test_invalid_creator_account_id(self):
        for domain in INVALID_DOMAINS:
            for name in VALID_NAMES_1:
                with self.assertRaises(ValueError):
                    self.builder.creatorAccountId("{}@{}".format(name, domain)).build()

        for domain in VALID_DOMAINS:
            for name in INVALID_NAMES_1:
                with self.assertRaises(ValueError):
                    self.builder.creatorAccountId("{}@{}".format(name, domain)).build()

    def test_valid_created_time(self):
        query = self.builder.createdTime(int(time.time() * 1000)).build()
        self.assertTrue(self.check_proto_query(self.proto(query)))



    def test_outdated_created_time(self):
        for i in [0, int((time.time() - 100000) * 1000), int((time.time() + 100000) * 1000)]:
            with self.assertRaises(ValueError):
                self.builder.createdTime(i).build()

    # ====================== BlocksQuery Tests ======================

if __name__ == '__main__':
    unittest.main()
