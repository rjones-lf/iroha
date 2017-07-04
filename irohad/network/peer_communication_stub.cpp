/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#include <network/peer_communication_stub.hpp>

namespace iroha {
  namespace network {

    using ametsuchi::Ametsuchi;
    using validation::StatefulValidator;
    using validation::ChainValidator;
    using ordering::OrderingService;
    using consensus::ConsensusService;
    using dao::Block;

    rxcpp::observable<rxcpp::observable<dao::Block>>
    PeerCommunicationServiceStub::on_commit() {
      return consensus_.on_commit().take_while([this](auto commit) {
        std::cout << "[PCS] chain validation" << std::endl;
        auto storage = storage_.createMutableStorage();
        auto result = chain_validator_.validate(commit, *storage);
        if (result) {
          storage_.commit(*storage);
        }
        return result;
      });
    }

    void PeerCommunicationServiceStub::propagate_transaction(
        const dao::Transaction &tx) {
      std::cout << "[PCS] sharing tx to ordering service" << std::endl;
      orderer_.propagate_transaction(tx);
    }

    rxcpp::observable<dao::Proposal>
    PeerCommunicationServiceStub::on_proposal() {
      return orderer_.on_proposal();
    }

    PeerCommunicationServiceStub::PeerCommunicationServiceStub(
        Ametsuchi &storage, StatefulValidator &stateful_validator,
        ChainValidator &chain_validator, OrderingService &orderer,
        ConsensusService &consensus, dao::DaoCryptoProvider &crypto_provider)
        : storage_(storage),
          stateful_validator_(stateful_validator),
          chain_validator_(chain_validator),
          orderer_(orderer),
          consensus_(consensus),
          crypto_provider_(crypto_provider) {
    }

    void PeerCommunicationServiceStub::subscribe_on_proposal() {
      on_proposal().subscribe([this](auto proposal) {
        std::cout << "[PCS] stateful validation" << std::endl;
        auto wsv = storage_.createTemporaryWsv();
        auto validated_proposal = stateful_validator_.validate(proposal, *wsv);
        Block block;
        std::for_each(validated_proposal.transactions.begin(),
                      validated_proposal.transactions.end(),
                      [&block](const auto &transaction) {
                        block.transactions.push_back(transaction);
                      });
        // TODO hash and sign
        consensus_.vote_block(block);
      });
    }
  }
}