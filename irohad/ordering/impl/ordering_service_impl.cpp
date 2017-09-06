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

#include "ordering/impl/ordering_service_impl.hpp"

/**
 * Will be published when transaction is received.
 */
struct TransactionEvent {};

namespace iroha {
  namespace ordering {
    OrderingServiceImpl::OrderingServiceImpl(
        std::shared_ptr<ametsuchi::PeerQuery> wsv, size_t max_size,
        size_t delay_milliseconds,
        std::shared_ptr<network::OrderingServiceTransport> transport,
        std::shared_ptr<uvw::Loop> loop)
        : loop_(std::move(loop)),
          timer_(loop_->resource<uvw::TimerHandle>()),
          wsv_(wsv),
          transport_(transport),
          max_size_(max_size),
          delay_milliseconds_(delay_milliseconds),
          proposal_height(2){

      timer_->on<uvw::TimerEvent>([this](const auto &, auto &) {
        if (!queue_.empty()) {
          this->generateProposal();
        }
        timer_->start(uvw::TimerHandle::Time(delay_milliseconds_),
                      uvw::TimerHandle::Time(0));
      });

      this->on<TransactionEvent>([this](const auto &, auto &) {
        if (queue_.unsafe_size() >= max_size_) {
          timer_->stop();
          this->generateProposal();
          timer_->start(uvw::TimerHandle::Time(delay_milliseconds_),
                        uvw::TimerHandle::Time(0));
        }
      });

      timer_->start(uvw::TimerHandle::Time(delay_milliseconds_),
                    uvw::TimerHandle::Time(0));
    }

    grpc::Status OrderingServiceImpl::SendTransaction(
        ::grpc::ServerContext *context, const protocol::Transaction *request,
        ::google::protobuf::Empty *response) {
      handleTransaction(std::move(*factory_.deserialize(*request)));

      return grpc::Status::OK;
    }

    void OrderingServiceImpl::handleTransaction(
        model::Transaction &&transaction) {
      queue_.push(transaction);

      publish(TransactionEvent{});
    }

    void OrderingServiceImpl::generateProposal() {
      auto txs = decltype(std::declval<model::Proposal>().transactions)();
      for (model::Transaction tx;
           txs.size() < max_size_ and queue_.try_pop(tx);) {
        txs.push_back(std::move(tx));
      }

      model::Proposal proposal(txs);
      proposal.height = proposal_height++;

      publishProposal(std::move(proposal));
    }

    void OrderingServiceImpl::publishProposal(model::Proposal &&proposal) {
      preparePeersForProposalRound();
      auto pb_proposal = std::make_shared<proto::Proposal>();
      pb_proposal->set_height(proposal.height);
      for (const auto &tx : proposal.transactions) {
        auto pb_tx = pb_proposal->add_transactions();
        new(pb_tx) protocol::Transaction(factory_.serialize(tx));
      }

      for (const auto &peer : peers_) {
        transport_->publishProposal(pb_proposal, peer.second);

      }
    }

    void OrderingServiceImpl::preparePeersForProposalRound() {
      peers_.clear();
      auto round_peers = wsv_->getLedgerPeers();
      if (not round_peers.has_value()) {
        // todo log error
        return;
      }
      for (const auto &peer : round_peers.value()) {
        peers_[peer.address] = proto::OrderingGate::NewStub(grpc::CreateChannel(
            peer.address, grpc::InsecureChannelCredentials()));
      }
    }

    OrderingServiceImpl::~OrderingServiceImpl() { timer_->close(); }
  }  // namespace ordering
}  // namespace iroha
