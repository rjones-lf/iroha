/*
 * Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
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
#include <service/flatbuffer_service.h>
#include <crypto/base64.hpp>
#include <crypto/hash.hpp>
#include <crypto/signature.hpp>
#include <infra/config/iroha_config_with_json.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <membership_service/peer_service.hpp>
#include <thread_pool.hpp>
#include <utils/explore.hpp>
#include <utils/logger.hpp>
#include <utils/timer.hpp>

#include <atomic>
#include <cmath>
#include <deque>
#include <map>
#include <queue>
#include <string>
#include <thread>

#include "connection/connection.hpp"
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
namespace sumeragi {

using iroha::ConsensusEvent;
using iroha::Signature;
using iroha::Transaction;

std::map<std::string, std::string> txCache;

static ThreadPool pool(ThreadPoolOptions{
    .threads_count =
        config::IrohaConfigManager::getInstance().getConcurrency(0),
    .worker_queue_size =
        config::IrohaConfigManager::getInstance().getPoolWorkerQueueSize(1024),
});

namespace detail {

std::string hash(const Transaction& tx) {
  return hash::sha3_256_hex(flatbuffer_service::toString(tx));
};

bool eventSignatureIsEmpty(const ::iroha::ConsensusEvent& event) {
  if (event.peerSignatures() != nullptr) {
    return event.peerSignatures()->size() == 0;
  } else {
    return 0;
  }
}

}  // namespace detail

struct Context {
  bool isSumeragi = false;      // am I the leader or am I not?
  std::uint64_t maxFaulty = 0;  // f
  std::uint64_t proxyTailNdx = 0;
  std::int32_t panicCount = 0;
  std::int64_t commitedCount = 0;
  std::uint64_t numValidatingPeers = 0;
  std::string myPublicKey;
  std::string myPrivateKey;
  std::string myIp;
  std::deque<std::unique_ptr<peer::Node>> validatingPeers;

  explore::sumeragi::PrintProgress printProgress;

  Context() { update(); }

  Context(std::vector<std::unique_ptr<peer::Node>>&& peers) {
    for (auto&& p : peers) {
      validatingPeers.push_back(std::move(p));
    }
  }

  void update() {
    auto peers = config::PeerServiceConfig::getInstance().getGroup();
    for (const auto& p : peers) {
      validatingPeers.push_back(std::make_unique<peer::Node>(
          p["ip"].get<std::string>(), p["publicKey"].get<std::string>()));
      logger::info("sumeragi")
          << "Add " << p["ip"].get<std::string>() << " to peerList";
    }

    this->numValidatingPeers = this->validatingPeers.size();
    // maxFaulty = Default to approx. 1/3 of the network.
    this->maxFaulty =
        config::IrohaConfigManager::getInstance().getMaxFaultyPeers(
            this->numValidatingPeers / 3);
    this->proxyTailNdx = this->maxFaulty * 2 + 1;

    if (this->validatingPeers.empty()) {
      logger::error("sumeragi") << "could not find any validating peers.";
      exit(EXIT_FAILURE);
    }
    logger::info("sumeragi") << "peerList is not empty";

    if (this->proxyTailNdx >= this->validatingPeers.size()) {
      this->proxyTailNdx = this->validatingPeers.size() - 1;
    }

    this->panicCount = 0;

    this->myPublicKey =
    config::PeerServiceConfig::getInstance().getMyPublicKey();

    this->myIp =
      config::PeerServiceConfig::getInstance().getMyIp();
    this->myPrivateKey =
      config::PeerServiceConfig::getInstance().getMyPrivateKey();

    this->isSumeragi =
        this->validatingPeers.at(0)->publicKey == this->myPublicKey;
    logger::info("sumeragi") << "update finished";

    this->printProgress.MAX = 100;
  }
};

std::unique_ptr<Context> context = nullptr;

void initializeSumeragi() {
  logger::info("sumeragi") << "Sumeragi setted";
  logger::info("sumeragi") << "set number of validatingPeer";

  context = std::make_unique<Context>();

  connection::iroha::SumeragiImpl::Torii::receive(
      [](const std::string& from, flatbuffers::unique_ptr_t&& transaction) {
        context->printProgress.print(1, "receive transaction!");

          flatbuffers::unique_ptr_t eventUniqPtr(
            flatbuffer_service::toConsensusEvent(
                *flatbuffers::GetRoot<::iroha::Transaction>(transaction.get())).value_move()
            );

          context->printProgress.print( 2, "make tx consensusEvent");

        if (eventUniqPtr != nullptr) {
          context->printProgress.print(2, "make tx consensusEvent");
          flatbuffers::unique_ptr_t ptr;
          // send processTransaction(event) as a task to processing pool
          // this returns std::future<void> object
          // (std::future).get() method locks processing until result of
          // processTransaction will be available but processTransaction returns
          // void, so we don't have to call it and wait
          auto&& task = [e = std::move(eventUniqPtr)]() mutable {
            processTransaction(std::move(e));
          };
          context->printProgress.print(3, "send event to processTransaction");
          pool.process(std::move(task));
        } else {
          //logger::error("sumeragi") << eventUniqPtr.error();
        }
      });

  connection::iroha::SumeragiImpl::Verify::receive([](
      const std::string& from, flatbuffers::unique_ptr_t&& eventUniqPtr) {
      context->printProgress.print( 16, "receive transaction form other sumeragi");
      auto eventPtr =
          flatbuffers::GetRoot<::iroha::ConsensusEvent>(eventUniqPtr.get());

      if (eventPtr->code() == iroha::Code::COMMIT) {
        context->printProgress.print( 20, "receive commited event");
        if (txCache.find(detail::hash(*eventPtr->transactions()->Get(0)->tx_nested_root())) ==
            txCache.end()) {
          // ToDo executor
          txCache[detail::hash(*eventPtr->transactions()->Get(0)->tx_nested_root())] = "commited";
        }
      } else {
        // send processTransaction(event) as a task to processing pool
        // this returns std::future<void> object
        // (std::future).get() method locks processing until result of
        // processTransaction will be available but processTransaction returns
        // void, so we don't have to call it and wait
        // std::function<void()>&& task =
        //    std::bind(processTransaction, std::move(event));
        // pool.process(std::move(task));

        // Copy ConsensusEvent
        auto&& task = [e = std::move(eventUniqPtr)]() mutable {
          processTransaction(std::move(e));
        };
        pool.process(std::move(task));
      }
  });

  logger::info("sumeragi") << "initialize numValidatingPeers :"
                           << context->numValidatingPeers;
  logger::info("sumeragi") << "initialize maxFaulty :" << context->maxFaulty;
  logger::info("sumeragi") << "initialize proxyTailNdx :"
                           << context->proxyTailNdx;

  logger::info("sumeragi") << "initialize panicCount :" << context->panicCount;
  logger::info("sumeragi") << "initialize myPublicKey :"
                           << context->myPublicKey;

  // TODO: move the peer service and ordering code to another place
  // determineConsensusOrder(); // side effect is to modify validatingPeers
  logger::info("sumeragi") << "initialize is sumeragi :"
                           << static_cast<int>(context->isSumeragi);
  logger::info("sumeragi") << "initialize.....  complete!";
}


std::uint64_t getNextOrder() {
  return 0l;
  // ToDo
  // return merkle_transaction_repository::getLastLeafOrder() + 1;
}

void processTransaction(flatbuffers::unique_ptr_t&& eventUniqPtr) {
    // Do not touch directly
    flatbuffers::unique_ptr_t storageUniqPtr;
    ::iroha::ConsensusEvent* storageRawPtrRef;

    // Helper to set unique_ptr_t
    auto resetUniqPtr = [&](flatbuffers::unique_ptr_t&& uptr) {
        storageUniqPtr = std::move(uptr);
        storageRawPtrRef = flatbuffers::GetMutableRoot<::iroha::ConsensusEvent>(
                storageUniqPtr.get());
    };

    // Convenient accessor
    auto getRoot = [&] { return storageRawPtrRef; };

  context->printProgress.print( 4, "start processTransaction");

  context->printProgress.print( 5, "convert eventUniqPtr to eventPtr");
  resetUniqPtr(std::move(eventUniqPtr));

  context->printProgress.print( 6, "generate hash");
  const auto hash = detail::hash(*getRoot()->transactions()->Get(0)->tx_nested_root());

  context->printProgress.print( 7, "sign hash using my key-pair");
  const auto signature = signature::sign(
    hash,
    context->myPublicKey,
    context->myPrivateKey
  );
  explore::sumeragi::printInfo("hash:" + hash + " signature:" + signature);

  context->printProgress.print( 8, "Add own signature");
  auto newEventUniqPtr = flatbuffer_service::addSignature(
      *getRoot(),
      context->myPublicKey,
      signature
  );

  context->printProgress.print(9, "if statement");
  if (detail::eventSignatureIsEmpty(*getRoot()) && context->isSumeragi) {
    context->printProgress.print(
        11, "event doesn't have signature and I'm Sumeragi");

    // Determine the order for processing this event
    // event.set_order(getNextOrder());//TODO getNexOrder is always return 0l;
    // logger::info("sumeragi") << "new  order:" << event.order();
  } else if (!detail::eventSignatureIsEmpty(*getRoot())) {
    context->printProgress.print(10, "event has signature");
    explore::sumeragi::printInfo(
        "Signature number is " +
        std::to_string(getRoot()->peerSignatures()->size()));

    context->printProgress.print(11, "if statement");
    // Check if we have at least 2f+1 signatures needed for Byzantine fault
    // tolerance ToDo re write
    if (getRoot()->peerSignatures()->size() >= context->maxFaulty * 2 + 1) {
      explore::sumeragi::printInfo("Signature exists and sig > 2*f + 1");

      explore::sumeragi::printJudge(getRoot()->peerSignatures()->size(),
                                    context->numValidatingPeers,
                                    context->maxFaulty * 2 + 1);
      explore::sumeragi::printAgree();

      context->printProgress.print(16, "commit");
      context->commitedCount++;

      explore::sumeragi::printInfo("commit count:" +
                                   std::to_string(context->commitedCount));

      context->printProgress.print(17, "update event commit");
      resetUniqPtr(flatbuffer_service::makeCommit(*getRoot()));

      context->printProgress.print(18, "SendAll");
      connection::iroha::SumeragiImpl::Verify::sendAll(*getRoot());

    } else {
      explore::sumeragi::printInfo("Signature exists and sig not enough");
      context->printProgress.print( 13, "make eventPtr event");

      explore::sumeragi::printInfo("tail public key is " + context->validatingPeers.at(context->proxyTailNdx)->publicKey);

      context->printProgress.print( 14, "If statements [ Am I tail or not?");
      if (context->validatingPeers.at(context->proxyTailNdx)->publicKey == context->myPublicKey
      ) {
        explore::sumeragi::printInfo("currently signature number:" + std::to_string(getRoot()->peerSignatures()->size()));
        context->printProgress.print( 15, "send to " + context->validatingPeers.at(context->proxyTailNdx)->ip);

        connection::iroha::SumeragiImpl::Verify::send(
            context->validatingPeers.at(context->proxyTailNdx)->ip,
            *getRoot());  // Think In Process
      } else {
        explore::sumeragi::printInfo(
            "currently signature number:" +
            std::to_string(getRoot()->peerSignatures()->size()));

        context->printProgress.print(14, "send all");
        connection::iroha::SumeragiImpl::Verify::sendAll(*getRoot());
        //
      }

      timer::setAwkTimerForCurrentThread(3000, [&]() { panic(*getRoot()); });
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
 * the set of considered validators, A, is expanded by f (e.g., by 1 in the
 * example below):
 *  ________________________    __________
 * /           A            \  /    B     \
 * |---|  |---|  |---|  |---|  |---|  |---|
 * | 0 |--| 1 |--| 2 |--| 3 |--| 4 |--| 5 |
 * |---|  |---|  |---|  |---|  |---|  |---|.
 */
void panic(const ConsensusEvent& event) {
  context->panicCount++;  // TODO: reset this later
  auto broadcastStart =
      2 * context->maxFaulty + 1 + context->maxFaulty * context->panicCount;
  auto broadcastEnd = broadcastStart + context->maxFaulty;

  // Do some bounds checking
  if (broadcastStart > context->numValidatingPeers - 1) {
    broadcastStart = context->numValidatingPeers - 1;
  }

  if (broadcastEnd > context->numValidatingPeers - 1) {
    broadcastEnd = context->numValidatingPeers - 1;
  }

  logger::info("sumeragi") << "broadcastEnd:" << broadcastEnd;
  logger::info("sumeragi") << "broadcastStart:" << broadcastStart;
  // WIP issue hash event
  // connection::sendAll(event->transaction().hash()); //TODO: change this to
  // only broadcast to peer range between broadcastStart and broadcastEnd
}

/**
 * The consensus order is based primarily on the trust scores. If two trust
 * scores are the same, then the order (ascending) of the public keys for the
 * servers are used.
 */
void determineConsensusOrder() {
  // WIP We create getTrustScore() function. till then circle list
  /*
  std::deque<
          std::unique_ptr<peer::Node>
  > tmp_deq;
  for(int i=1;i<context->validatingPeers.size();i++){
      tmp_deq.push_back(std::move(context->validatingPeers[i]));
  }
  tmp_deq.push_back(std::move(context->validatingPeers[0]));
  context->validatingPeers.clear();
  context->validatingPeers = std::move(tmp_deq);

  std::sort(context->validatingPeers.begin(), context->validatingPeers.end(),
        [](const std::unique_ptr<peer::Node> &lhs,
           const std::unique_ptr<peer::Node> &rhs) {
            return lhs->getTrustScore() > rhs->getTrustScore()
                   || (lhs->getTrustScore() == rhs->getTrustScore()
                       && lhs->getPublicKey() < rhs->getPublicKey());
        }
  );
  logger::info("sumeragi")        <<  "determineConsensusOrder sorted!";
  logger::info("sumeragi")        <<  "determineConsensusOrder myPubkey:"     <<
  context->myPublicKey;

  for(const auto& peer : context->validatingPeers) {
      logger::info("sumeragi")    <<  "determineConsensusOrder PublicKey:"    <<
  peer->getPublicKey(); logger::info("sumeragi")    <<  "determineConsensusOrder
  ip:"           <<  peer->getIP();
  }
  */
  // context->isSumeragi = context->validatingPeers.at(0)->getPublicKey() ==
  // context->myPublicKey;
}
}  // namespace sumeragi
