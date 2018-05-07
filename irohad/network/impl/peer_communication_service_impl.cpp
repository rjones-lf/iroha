/*
Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.

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

#include "network/impl/peer_communication_service_impl.hpp"

namespace iroha {
  namespace network {
    PeerCommunicationServiceImpl::PeerCommunicationServiceImpl(
        std::shared_ptr<OrderingGate> ordering_gate,
        std::shared_ptr<synchronizer::Synchronizer> synchronizer)
        : ordering_gate_(std::move(ordering_gate)),
          synchronizer_(std::move(synchronizer)) {
      log_ = logger::log("PCS");
    }

    void PeerCommunicationServiceImpl::propagate_transaction(
        std::shared_ptr<const shared_model::interface::Transaction>
            transaction) {
      log_->info("propagate tx");
      ordering_gate_->propagateTransaction(transaction);
    }

    rxcpp::observable<std::shared_ptr<shared_model::interface::Proposal>>
    PeerCommunicationServiceImpl::on_proposal() const {
      return ordering_gate_->on_proposal();
    }

    rxcpp::observable<Commit> PeerCommunicationServiceImpl::on_commit() const {
      return synchronizer_->on_commit_chain();
    }
  }  // namespace network
}  // namespace iroha
