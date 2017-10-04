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

#include "consensus/yac/transport/impl/network_impl.hpp"

#include <grpc++/grpc++.h>

#include "consensus/yac/transport/yac_pb_converters.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      // ----------| Public API |----------

      NetworkImpl::NetworkImpl(const std::string &address,
                               const std::vector<model::Peer> &peers)
          : address_(address) {
        for (const auto &peer : peers) {
          peers_[peer] = proto::Yac::NewStub(grpc::CreateChannel(
              peer.address, grpc::InsecureChannelCredentials()));
          peers_addresses_[peer.address] = peer;
        }
        log_ = logger::log("YacNetwork");
      }

      void NetworkImpl::subscribe(
          std::shared_ptr<YacNetworkNotifications> handler) {
        handler_ = handler;
      }

      void NetworkImpl::send_vote(model::Peer to, VoteMessage vote) {
        auto request = PbConverters::serializeVote(vote);

        auto call = new AsyncClientCall;

        call->context.AddMetadata("address", address_);

        call->response_reader =
            peers_.at(to)->AsyncSendVote(&call->context, request, &cq_);

        call->response_reader->Finish(&call->reply, &call->status, call);

        log_->info("Send vote {} to {}", vote.hash.block_hash, to.address);
      }

      void NetworkImpl::send_commit(model::Peer to, CommitMessage commit) {
        proto::Commit request;
        for (const auto &vote : commit.votes) {
          auto pb_vote = request.add_votes();
          *pb_vote = PbConverters::serializeVote(vote);
        }

        auto call = new AsyncClientCall;

        call->context.AddMetadata("address", address_);

        call->response_reader =
            peers_.at(to)->AsyncSendCommit(&call->context, request, &cq_);

        call->response_reader->Finish(&call->reply, &call->status, call);

        log_->info("Send votes bundle[size={}] commit to {}",
                   commit.votes.size(),
                   to.address);
      }

      void NetworkImpl::send_reject(model::Peer to, RejectMessage reject) {
        proto::Reject request;
        for (const auto &vote : reject.votes) {
          auto pb_vote = request.add_votes();
          *pb_vote = PbConverters::serializeVote(vote);
        }

        auto call = new AsyncClientCall;

        call->context.AddMetadata("address", address_);

        call->response_reader =
            peers_.at(to)->AsyncSendReject(&call->context, request, &cq_);

        call->response_reader->Finish(&call->reply, &call->status, call);

        log_->info("Send votes bundle[size={}] reject to {}",
                   reject.votes.size(),
                   to.address);
      }

      grpc::Status NetworkImpl::SendVote(
          ::grpc::ServerContext *context,
          const ::iroha::consensus::yac::proto::Vote *request,
          ::google::protobuf::Empty *response) {
        auto it = context->client_metadata().find("address");
        if (it == context->client_metadata().end()) {
          log_->error("Missing source address");
          // TODO handle missing source address
          return grpc::Status::CANCELLED;
        }
        auto address = std::string(it->second.data(), it->second.size());
        auto peer = peers_addresses_.at(address);

        auto vote = *PbConverters::deserializeVote(*request);

        log_->info(
            "Receive vote {} from {}", vote.hash.block_hash, peer.address);

        handler_.lock()->on_vote(peer, vote);
        return grpc::Status::OK;
      }

      grpc::Status NetworkImpl::SendCommit(
          ::grpc::ServerContext *context,
          const ::iroha::consensus::yac::proto::Commit *request,
          ::google::protobuf::Empty *response) {
        auto it = context->client_metadata().find("address");
        if (it == context->client_metadata().end()) {
          log_->error("Missing source address");
          // TODO handle missing source address
          return grpc::Status::CANCELLED;
        }
        auto address = std::string(it->second.data(), it->second.size());
        auto peer = peers_addresses_.at(address);

        CommitMessage commit;
        for (const auto &pb_vote : request->votes()) {
          auto vote = *PbConverters::deserializeVote(pb_vote);
          commit.votes.push_back(vote);
        }

        log_->info("Receive commit[size={}] from {}",
                   commit.votes.size(),
                   peer.address);

        handler_.lock()->on_commit(peer, commit);
        return grpc::Status::OK;
      }

      grpc::Status NetworkImpl::SendReject(
          ::grpc::ServerContext *context,
          const ::iroha::consensus::yac::proto::Reject *request,
          ::google::protobuf::Empty *response) {
        auto it = context->client_metadata().find("address");
        if (it == context->client_metadata().end()) {
          log_->error("Missing source address");
          // TODO handle missing source address
          return grpc::Status::CANCELLED;
        }
        auto address = std::string(it->second.data(), it->second.size());
        auto peer = peers_addresses_.at(address);

        RejectMessage reject;
        for (const auto &pb_vote : request->votes()) {
          auto vote = *PbConverters::deserializeVote(pb_vote);
          reject.votes.push_back(vote);
        }

        log_->info("Receive reject[size={}] from {}",
                   reject.votes.size(),
                   peer.address);

        handler_.lock()->on_reject(peer, reject);
        return grpc::Status::OK;
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
