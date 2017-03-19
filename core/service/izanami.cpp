/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
     http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include "izanami.hpp"
#include "executor.hpp"

#include <consensus/connection/connection.hpp>
#include <crypto/hash.hpp>
#include <infra/config/iroha_config_with_json.hpp>
#include <infra/config/peer_service_with_json.hpp>
#include <repository/transaction_repository.hpp>
#include <service/peer_service.hpp>
#include <thread_pool.hpp>

namespace peer {
  namespace izanami {
    using Api::TransactionResponse;

    void setAwkTimer(int const sleepMillisecs,
                     const std::function<void(void)> &action) {
      std::thread([action, sleepMillisecs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMillisecs));
        action();
      }).join();
    }

    InitializeEvent::InitializeEvent() {
      now_progress = 0;
      ::peer::myself::stop();
    }

    void InitializeEvent::add_transactionResponse(
        std::unique_ptr<TransactionResponse> txResponse) {
      if (now() > txResponse->code())
        return; // management index of progress is TransactionResponse.code()

      // make TransactionResponse hash - temporary
      std::string hash = "";
      for (auto &&tx : txResponse->transaction()) {
        hash = hash::sha3_256_hex(hash + tx.hash());
      }
      hashes[txResponse->code()].emplace_back(hash);
      txResponses[hash] = std::move(txResponse);
    }

    const std::vector<std::string> &InitializeEvent::getHashes(uint64_t progress) {
      return hashes[progress];
    }

    const std::unique_ptr<TransactionResponse>
    InitializeEvent::getTransactionResponse(const std::string &hash) {
      return std::move(txResponses[hash]);
    }

    void InitializeEvent::next_progress() {
      logger::debug("izanami") << "next_progress : " << std::to_string(now());
      for (auto &&hash : hashes[now()]) {
        txResponses.erase(hash);
      }
      logger::debug("izanami") << "txResponses erase";
      hashes.erase(now());
      now_progress++;
      logger::debug("izanami") << "nexted : " << std::to_string(now());
    }

    uint64_t InitializeEvent::now() const { return now_progress; }

    void InitializeEvent::storeTxResponse(const std::string &hash) {
      for (auto &&tx : txResponses[hash]->transaction()) {
        // WIP repository::transaction::add( tx.hash(), tx );
      }
    }

    void InitializeEvent::executeTxResponse(const std::string &hash) {
      for (auto &&tx : txResponses[hash]->transaction()) {
        executor::execute(std::move(tx));
      }
    }

    bool InitializeEvent::isExistTransactionFromHash(const std::string &hash) {
      for (auto &&tx : txResponses[hash]->transaction())
        return true;
      return false;
    }

    bool InitializeEvent::isFinished() const { return is_finished; }

    void InitializeEvent::finish() {
      now_progress = 0;
      txResponses.clear();
      hashes.clear();
      is_finished = true;
    }

    namespace detail {
      bool isFinishedReceiveAll(InitializeEvent &event) {
        std::string hash = getCorrectHash(event);
        if (event.isExistTransactionFromHash(hash))
          return false;
        return true;
      }

      bool isFinishedReceive(InitializeEvent &event) {
        std::unordered_map<std::string, std::size_t> hash_counter;
        for (std::string hash : event.getHashes(event.now())) {
          hash_counter[hash]++;
        }
        std::size_t res = 0;
        for (auto counter : hash_counter) {
          res = std::max(res, counter.second);
        }
        if (res >= 2 * ::peer::service::getMaxFaulty() + 1)
          return true;
        return false;
      }

      std::string getCorrectHash(InitializeEvent &event) {
        std::unordered_map<std::string, std::size_t> hash_counter;
        for (std::string hash : event.getHashes(event.now())) {
          hash_counter[hash]++;
        }
        std::size_t res = 0;
        std::string res_hash;
        for (auto counter : hash_counter) {
          if (res < counter.second) {
            res = counter.second;
            res_hash = counter.first;
          }
        }
        if (res >= 2 * ::peer::service::getMaxFaulty() + 1)
          return res_hash;
        return "";
      }

      void storeTransactionResponse(InitializeEvent &event) {
        std::string hash = getCorrectHash(event);
        event.storeTxResponse(hash);
        event.executeTxResponse(hash);
        event.next_progress();
      }
    }

// invoke when receive TransactionResponse.
    void receiveTransactionResponse(TransactionResponse &txResponse) {
      static InitializeEvent event;
      logger::debug("izanami") << "in receiveTransactionResponse event = "
                               << std::to_string(event.now());
      if (event.isFinished())
        return;
      logger::debug("izanami") << "event is not finished";
      event.add_transactionResponse(
          std::make_unique<TransactionResponse>(txResponse));
      if (detail::isFinishedReceive(event)) {
        logger::debug("izanami") << "is finished receive";
        if (detail::isFinishedReceiveAll(event)) {
          logger::debug("izanami") << "is finished receive all";
          ::peer::transaction::izanami::finished();
          event.finish();
          logger::explore("izanami") << "Finished Receive ALl Transaction";
          logger::explore("izanami") << "Closed Izanami";
          for (const auto &p : ::peer::service::getPeerList()) {
            logger::explore("izanami") << ("ip: " + p->ip);
            logger::explore("izanami") << ("pubkey: " + p->publicKey);
            logger::explore("izanami")
                << ("trust: " + std::to_string(p->trustScore));
          }
        } else {
          detail::storeTransactionResponse(event);
        }
      }
      if (!event.isFinished() && txResponse.transaction().empty())
        setAwkTimer(1000, [&txResponse]() {
          connection::iroha::PeerService::Izanami::send(::peer::myself::getIp(),
                                                        txResponse);
        });
    }

    static ThreadPool pool(ThreadPoolOptions {
      .threads_count =
           config::IrohaConfigManager::getInstance().getConcurrency(0),
      .worker_queue_size =
           config::IrohaConfigManager::getInstance().getPoolWorkerQueueSize(1024),
    }

    );

// invoke when initialize Peer that to config Participation on the way
    void startIzanami() {
      logger::explore("izanami") << "startIzanami";
      if (config::IrohaConfigManager::getInstance().getActiveStart(false)) {
        logger::explore("izanami") << "I am Active Start Iroha Peer.";
        logger::explore("izanami") << "Closed Izanami";
        return;
      }

      logger::explore("izanami") << "\033[95m+==ーーーーーーーーーー==+\033[0m";
      logger::explore("izanami") << "\033[95m|+-ーーーーーーーーーー-+|\033[0m";
      logger::explore("izanami") << "\033[95m||  　　　　　　　　　 ||\033[0m";
      logger::explore("izanami") << "\033[95m||初回取引履歴構築機構 ||\033[0m";
      logger::explore("izanami") << "\033[95m||　　　イザナミ　　　　||\033[0m";
      logger::explore("izanami") << "\033[95m|| 　　　　　　 　　　 ||\033[0m";
      logger::explore("izanami") << "\033[95m|+-ーーーーーーーーーー-+|\033[0m";
      logger::explore("izanami") << "\033[95m+==ーーーーーーーーーー==+\033[0m";
      logger::explore("izanami") << "- 起動/setup";

      logger::info("izanami") << "My PublicKey is "
                              << ::peer::myself::getPublicKey();
      logger::info("izanami") << "My key is " << ::peer::myself::getIp();

      connection::iroha::Izanami::Izanagi::receive(
          [](const std::string &from, TransactionResponse &txResponse) {
            logger::info("izanami") << "receive! Transactions!!";
            logger::info("izanami") << txResponse.message();
            std::function<void()> &&task =
                std::bind(receiveTransactionResponse, txResponse);
            pool.process(std::move(task));
          });
    }

  }
}
