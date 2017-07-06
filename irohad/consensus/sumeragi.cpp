/*
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
 * http://soramitsu.co.jp
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *          http://www.apache.org/licenses/LICENSE-2.0
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <crypto/crypto.hpp>
#include <validation/stateful/validator.hpp>
#include <torii/command_service.hpp>
#include <logger/logger.hpp>
#include <peer_service/self_state.hpp>
#include <peer_service/monitor.hpp>
#include <timer/timer.hpp>
#include <datetime/time.hpp>
#include <vector>
#include <set>
#include <common/types.hpp>
#include <common/byteutils.hpp>

#include "connection/service.hpp"
#include "connection/client.hpp"
#include "sumeragi.hpp"

/**
 * |ーーー|　|ーーー|　|ーーー|　|ーーー|
 * |　ス　|ー|　メ　|ー|　ラ　|ー|　ギ　|
 * |ーーー|　|ーーー|　|ーーー|　|ーーー|
 *
 * A chain-based byzantine fault tolerant consensus algorithm, based in large
 * part on BChain:
 *
 * Duan, S., Meling, H., Peisert, S., & Zhang, H. (2014). Bchain: Byzantine
 * replication with high throughput and embedded reconfiguration. In
 * International Conference on Principles of Distributed Systems (pp. 91-106).
 * Springer.
 */

namespace consensus {
  namespace sumeragi {

    using iroha::protocol::Block;
    using iroha::protocol::Signature;

    logger::Logger log("sumeragi");

    void initialize() {

    }

    size_t getMaxFaulty() {
      return (size_t) peer_service::monitor::getActivePeerSize() / 3;
    }

    size_t getNumValidatingPeers() {
      return getMaxFaulty() * 2 + 1;
    }

    bool unicast(const iroha::protocol::Block &block, size_t peerOrder) {
      auto peer =
          peer_service::monitor::getActivePeerAt((unsigned int) peerOrder);
      auto response = connection::sendBlock(block, peer->ip_);
      return response.code() == iroha::protocol::ResponseCode::OK;
    }

    bool leaderMulticast(const iroha::protocol::Block &block) {
      // connection::multicastWithRange(block, 1, getNumValidatingPeers());
      /*
      auto peerSize = getNumValidatingPeers();
      for (size_t i = 0; i < peerSize; i++) {
        unicast(block, i); // Currently, return value is not used.
      }
      */
      return true;
    }

    bool commit(const iroha::protocol::Block &block) {
      // connection::multicastAll(block);
      /*
      auto peerSize = (size_t)peer_service::monitor::getActivePeerSize();
      for (size_t i = 0; i < peerSize; i++) {
        unicast(block, i); // Currently, return value is not used.
      }
      */
      return true;
    }

    // TODO: Append block to db and calc merkle root.
    std::vector<uint8_t> appendBlock(const Block &block) {
      return std::vector<uint8_t>();
    }

    Block createSignedBlock(const Block &block,
                            const std::vector<uint8_t> &merkleRoot) {

      // TODO: Use Keypair in peer service.
      std::string pkBase64 = peer_service::self_state::getPublicKey();
      std::string skBase64 = peer_service::self_state::getPrivateKey();

      auto pubkey_ = base64_decode(pkBase64),
          privkey_ = base64_decode(skBase64);
      auto pubkey =
          iroha::to_blob<iroha::ed25519::pubkey_t::size()>(std::string{
              pubkey_.begin(), pubkey_.end()});
      auto privkey =
          iroha::to_blob<iroha::ed25519::privkey_t::size()>(std::string{
              privkey_.begin(), privkey_.end()});

      auto signature =
          iroha::sign(merkleRoot.data(), merkleRoot.size(), pubkey, privkey);

      std::string strSigblob;
      for (auto e: signature) strSigblob.push_back(e);

      Signature newSignature;
      *newSignature.mutable_pubkey() = pubkey.to_base64();
      *newSignature.mutable_signature() = strSigblob;

      Block ret;
      ret.CopyFrom(block);
      ret.mutable_header()->set_created_time(iroha::time::now64());
      *ret.mutable_header()->mutable_peer_signature()->Add() = newSignature;

      return ret;
    }

    void setTimeOutCommit(const Block &block) {
      timer::setAwkTimerForCurrentThread(3000, [block] {
        panic(block);
      });
    }

    /**
     * returns expected tail to send committed block.
     * if returned value = -1, all peers has been used.
     */

    int getNextOrder() {
      thread_local int
          currentProxyTail = static_cast<int>(getNumValidatingPeers()) - 1;
      if (currentProxyTail >= peer_service::monitor::getActivePeerSize()) {
        return -1;
      }
      return currentProxyTail++;
    }

    size_t countValidSignatures(const Block &block) {
      size_t numValidSignatures = 0;
      std::set<std::string> usedPubkeys;

      auto peerSigs = block.header().peer_signature();
      for (auto const &sig: peerSigs) {
        // FIXME: bytes in proto -> std::string in C++ (null value problem)
        if (usedPubkeys.count(sig.pubkey())) continue;
        const auto bodyMessage = block.body().SerializeAsString();

        // TODO: Use new Keypair class.
        /*
        const auto hash = iroha::hash::sha3_256_hex(bodyMessage);
        if (iroha::signature::verify(sig.signature(), hash, sig.pubkey())) {
          numValidSignatures++;
          usedPubkeys.insert(sig.pubkey());
        }
         */
      }

      return numValidSignatures;
    }

    void processBlock(const Block &block) {

      // Stateful Validation
//      auto valid = validaton::stateful::validate(block);
//      if (!valid) {
//        log.info("Stateful validation failed.");
//        return;
//      }

      // Add Signature
      auto merkleRoot = appendBlock(block);
      auto newBlock = createSignedBlock(block, merkleRoot);

      if (peer_service::self_state::isLeader()) {
        leaderMulticast(newBlock);
        setTimeOutCommit(newBlock);
        return;
      }

      auto numValidSignatures = countValidSignatures(newBlock);

      if (numValidSignatures < getNumValidatingPeers()) {
        auto next = getNextOrder();
        if (next < 0) {
          log.error("getNextOrder() < 0 in processBlock");
          return;
        }
        unicast(newBlock, static_cast<size_t>(next));
        setTimeOutCommit(newBlock);
      } else {
        if (numValidSignatures == getNumValidatingPeers()) {
          commit(newBlock);
          setTimeOutCommit(newBlock);
        }
      }
    }

/**
 *
 * For example, given:
 * if f := 1, then
 *  _________________    _________________
 * /        A        \  /        B        \
 * |---|  |---|  |---|  |---|  |---|  |---|
 * | 0 |--| 1 |--| 2 |--| 3 |--| 4 |--| 5 |
 * |---|  |---|  |---|  |---|  |---|  |---|,
 *
 * if 2f+1 signature are not received within the timer's limit, then
 * the set of considered validators, A, is expanded by 1.
 *  ________________________    __________
 * /           A            \  /    B     \
 * |---|  |---|  |---|  |---|  |---|  |---|
 * | 0 |--| 1 |--| 2 |--| 3 |--| 4 |--| 5 |
 * |---|  |---|  |---|  |---|  |---|  |---|.
 */

    void panic(const Block &block) {
      auto next = getNextOrder();
      if (next < 0) {
        log.info("否認");
        return;
      }
      unicast(block, static_cast<size_t>(next));
      setTimeOutCommit(block);
    }

  }  // namespace sumeragi
}  // namespace consensus
