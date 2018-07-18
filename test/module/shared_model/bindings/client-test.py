# 
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
# 

from time import time
import unittest

import transaction_pb2 as trx
import commands_pb2 as cmd
import iroha

class ClientTest(unittest.TestCase):
  def setUp(self):
    self.keys = iroha.ModelCrypto().generateKeypair()

  def valid_add_peer_command(self):
    command = cmd.Command()
    command.add_peer.peer.address = "127.0.0.1:50500"
    command.add_peer.peer.peer_key = b'A' * 32
    return command

  def unsigned_tx(self):
    tx = trx.Transaction()
    tx.payload.reduced_payload.creator_account_id = "admin@test"
    tx.payload.reduced_payload.created_time = int(time() * 1000)
    tx.payload.reduced_payload.quorum = 1
    return tx

  def test_hash(self):
    tx = self.unsigned_tx()
    tx.payload.reduced_payload.commands.extend([self.valid_add_peer_command()])
    h = iroha.hashTransaction(iroha.Blob(tx.SerializeToString()).blob())
    self.assertEqual(len(h), 32)

  def test_sign(self):
    tx = self.unsigned_tx()
    tx.payload.reduced_payload.commands.extend([self.valid_add_peer_command()])
    self.assertEqual(len(tx.signatures), 0)
    tx_blob = iroha.signTransaction(iroha.Blob(tx.SerializeToString()).blob(), self.keys)
    signed_tx = trx.Transaction()
    signed_tx.ParseFromString(bytearray(tx_blob))
    self.assertEqual(len(signed_tx.signatures), 1)

  def test_validate_without_cmd(self):
    tx = self.unsigned_tx()
    tx_blob = iroha.signTransaction(iroha.Blob(tx.SerializeToString()).blob(), self.keys)
    with self.assertRaises(ValueError):
      iroha.validateTransaction(tx_blob)

  def test_validate_unsigned_tx(self):
    tx = self.unsigned_tx()
    tx.payload.reduced_payload.commands.extend([self.valid_add_peer_command()])
    self.assertEqual(len(tx.signatures), 0)
    with self.assertRaises(ValueError):
      iroha.validateTransaction(iroha.Blob(tx.SerializeToString()).blob())

  def test_validate_correct_tx(self):
    tx = self.unsigned_tx()
    tx.payload.reduced_payload.commands.extend([self.valid_add_peer_command()])
    self.assertEqual(len(tx.signatures), 0)
    iroha.signTransaction(iroha.Blob(tx.SerializeToString()).blob(), self.keys)

if __name__ == '__main__':
  unittest.main()
