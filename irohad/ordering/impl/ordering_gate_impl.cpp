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

#include "ordering/impl/ordering_gate_impl.hpp"

namespace iroha {
  namespace ordering {

    OrderingGateImpl::OrderingGateImpl(std::shared_ptr<network::OrderingGateTransport> transport)
        : transport_(transport) {
      log_ = logger::log("OrderingGate");
    }

    void OrderingGateImpl::propagate_transaction(
        std::shared_ptr<const model::Transaction> transaction) {
      log_->info("propagate tx");
      transport_->propagate(transaction);
    }

    rxcpp::observable<model::Proposal> OrderingGateImpl::on_proposal() {
      return proposals_.get_observable();
    }

    grpc::Status OrderingGateImpl::SendProposal(
        ::grpc::ServerContext *context, const proto::Proposal *request,
        ::google::protobuf::Empty *response) {
      log_->info("receive proposal");
      // auto removes const qualifier of model::Proposal.transactions
      auto transactions =
          decltype(std::declval<model::Proposal>().transactions)();
      for (const auto &tx : request->transactions()) {
        transactions.push_back(*factory_.deserialize(tx));
      }
      log_->info("transactions in proposal: {}", transactions.size());

      auto proposal = transport_->getProposal(request);

      proposal->height = request->height();
      transport_->subscriber_->handleProposal(proposal);
      //handleProposal(proposal);
      return grpc::Status::OK;
    }

    void OrderingGateImpl::handleProposal(std::shared_ptr<model::Proposal> proposal) {
      proposals_.get_subscriber().on_next(*proposal);
    }
  }  // namespace ordering
}  // namespace iroha
