/**
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
 * http://soramitsu.co.jp
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <memory>
#include <crypto/signature.hpp>
#include <validation/transaction_validator.hpp>
#include <infra/protobuf/api.grpc.pb.h>

using Api::ConsensusEvent;
using Api::Transaction;
using Api::Signature;


std::string public_key_b64  = "slyr7oz2+EU6dh2dY9+jNeO/hVrXCkT3rGhcNZo5rrE=";
std::string hash = "46ed8c250356759f68930a94996faaa8f8c98ecbe0dcc58c479c8fad71e30096";
std::string signature_b64   = "gdMUgjyo++4QpF1xDJNdk1a5zmDAEPM67WD4cn6CVZqDxC8nShb/L1Tokgo53HSOPDB0qXAVzcBvfcJ1WLjrAQ==";

TEST(transaction_validator, verify_transaction_event) {
    Transaction tx;
    Signature sig;
    sig.set_publickey(public_key_b64);
    sig.set_signature(signature_b64);
    tx.set_hash(hash);
    tx.add_txsignatures()->CopyFrom(sig);
    ASSERT_EQ(transaction_validator::signaturesAreValid(tx),
        signature::verify(tx.txsignatures(0).signature(), tx.hash(), tx.txsignatures(0).publickey()));

    // Wrong hashes shouldn't be validated
    tx.set_hash("123");
    ASSERT_NE(transaction_validator::signaturesAreValid(tx),
        signature::verify(signature_b64, hash, public_key_b64));
}

TEST(transaction_validator, verify_consensus_event) {
    ConsensusEvent ev;
    auto tx = new Transaction;
    Signature sig;
    tx->set_hash(hash);
    sig.set_publickey(public_key_b64);
    sig.set_signature(signature_b64);
    ev.set_allocated_transaction(tx);
    ev.add_eventsignatures()->CopyFrom(sig);
    ASSERT_EQ(transaction_validator::signaturesAreValid(ev),
        signature::verify(ev.eventsignatures(0).signature(), ev.transaction().hash(), ev.eventsignatures(0).publickey()));

    // Wrong hashes shouldn't be validated
    tx->set_hash("123");
    ASSERT_NE(transaction_validator::signaturesAreValid(ev),
        signature::verify(signature_b64, hash, public_key_b64));
}
