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
#include <memory>

#include "consensus/yac/messages.hpp"
#include "consensus/yac/transport/yac_pb_converters.hpp"
#include "interfaces/common_objects/peer.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace consensus {
    namespace yac {
      // ----------| Public API |----------

      NetworkImpl::NetworkImpl()
          : network::AsyncGrpcClient<google::protobuf::Empty>(
                logger::log("YacNetwork")) {}

      void NetworkImpl::subscribe(
          std::shared_ptr<YacNetworkNotifications> handler) {
        handler_ = handler;
      }

      void NetworkImpl::send_vote(const shared_model::interface::Peer &to,
                                  VoteMessage vote) {
        createPeerConnection(to);

        auto request = PbConverters::serializeVote(vote);

        auto call = new AsyncClientCall;

        call->response_reader =
            peers_.at(to.address())
                ->AsyncSendVote(&call->context, request, &cq_);

        call->response_reader->Finish(&call->reply, &call->status, call);

        log_->info("Send vote {} to {}", vote.hash.block_hash, to.address());
      }

      void NetworkImpl::send_commit(const shared_model::interface::Peer &to,
                                    const CommitMessage &commit) {
        createPeerConnection(to);

        proto::Commit request;
        for (const auto &vote : commit.votes) {
          auto pb_vote = request.add_votes();
          *pb_vote = PbConverters::serializeVote(vote);
        }

        auto call = new AsyncClientCall;

        call->response_reader =
            peers_.at(to.address())
                ->AsyncSendCommit(&call->context, request, &cq_);

        call->response_reader->Finish(&call->reply, &call->status, call);

        log_->info("Send votes bundle[size={}] commit to {}",
                   commit.votes.size(),
                   to.address());
      }

      void NetworkImpl::send_reject(const shared_model::interface::Peer &to,
                                    RejectMessage reject) {
        createPeerConnection(to);

        proto::Reject request;
        for (const auto &vote : reject.votes) {
          auto pb_vote = request.add_votes();
          *pb_vote = PbConverters::serializeVote(vote);
        }

        auto call = new AsyncClientCall;

        call->response_reader =
            peers_.at(to.address())
                ->AsyncSendReject(&call->context, request, &cq_);

        call->response_reader->Finish(&call->reply, &call->status, call);

        log_->info("Send votes bundle[size={}] reject to {}",
                   reject.votes.size(),
                   to.address());
      }

      grpc::Status NetworkImpl::SendVote(
          ::grpc::ServerContext *context,
          const ::iroha::consensus::yac::proto::Vote *request,
          ::google::protobuf::Empty *response) {
        auto vote = *PbConverters::deserializeVote(*request);

        log_->info(
            "Receive vote {} from {}", vote.hash.block_hash, context->peer());

        handler_.lock()->on_vote(vote);
        return grpc::Status::OK;
      }

      grpc::Status NetworkImpl::SendCommit(
          ::grpc::ServerContext *context,
          const ::iroha::consensus::yac::proto::Commit *request,
          ::google::protobuf::Empty *response) {
        CommitMessage commit(std::vector<VoteMessage>{});
        for (const auto &pb_vote : request->votes()) {
          auto vote = *PbConverters::deserializeVote(pb_vote);
          commit.votes.push_back(vote);
        }

        log_->info("Receive commit[size={}] from {}",
                   commit.votes.size(),
                   context->peer());

        handler_.lock()->on_commit(commit);
        return grpc::Status::OK;
      }

      grpc::Status NetworkImpl::SendReject(
          ::grpc::ServerContext *context,
          const ::iroha::consensus::yac::proto::Reject *request,
          ::google::protobuf::Empty *response) {
        RejectMessage reject(std::vector<VoteMessage>{});
        for (const auto &pb_vote : request->votes()) {
          auto vote = *PbConverters::deserializeVote(pb_vote);
          reject.votes.push_back(vote);
        }

        log_->info("Receive reject[size={}] from {}",
                   reject.votes.size(),
                   context->peer());

        handler_.lock()->on_reject(reject);
        return grpc::Status::OK;
      }

      void NetworkImpl::createPeerConnection(
          const shared_model::interface::Peer &peer) {
        if (peers_.count(peer.address()) == 0) {
          peers_[peer.address()] = proto::Yac::NewStub(grpc::CreateChannel(
              peer.address(), grpc::InsecureChannelCredentials()));
        }
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
