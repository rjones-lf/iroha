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
#include "consensus/yac/storage/yac_common.hpp"
#include "consensus/yac/transport/yac_pb_converters.hpp"
#include "interfaces/common_objects/peer.hpp"
#include "logger/logger.hpp"
#include "network/impl/grpc_channel_builder.hpp"
#include "yac.pb.h"

namespace iroha {
  namespace consensus {
    namespace yac {
      // ----------| Public API |----------

      NetworkImpl::NetworkImpl(
          std::shared_ptr<network::AsyncGrpcClient<google::protobuf::Empty>>
              async_call)
          : async_call_(async_call) {}

      void NetworkImpl::subscribe(
          std::shared_ptr<YacNetworkNotifications> handler) {
        handler_ = handler;
      }

      void NetworkImpl::sendState(const shared_model::interface::Peer &to,
                                  const std::vector<VoteMessage> &state) {
        createPeerConnection(to);

        proto::State request;
        for (const auto &vote : state) {
          auto pb_vote = request.add_votes();
          *pb_vote = PbConverters::serializeVote(vote);
        }

        async_call_->Call([&](auto context, auto cq) {
          return peers_.at(to.address())->AsyncSendState(context, request, cq);
        });

        async_call_->log_->info(
            "Send votes bundle[size={}] to {}", state.size(), to.address());
      }

      grpc::Status NetworkImpl::SendState(
          ::grpc::ServerContext *context,
          const ::iroha::consensus::yac::proto::State *request,
          ::google::protobuf::Empty *response) {
        std::vector<VoteMessage> state;
        for (const auto &pb_vote : request->votes()) {
          auto vote = *PbConverters::deserializeVote(pb_vote);
          state.push_back(vote);
        }
        if (not sameKeys(state)) {
          async_call_->log_->info(
              "Votes are stateless invalid: proposals are different, or empty "
              "collection");
          return grpc::Status::CANCELLED;
        }

        async_call_->log_->info(
            "Receive votes[size={}] from {}", state.size(), context->peer());

        handler_.lock()->onState(state);
        return grpc::Status::OK;
      }

      void NetworkImpl::createPeerConnection(
          const shared_model::interface::Peer &peer) {
        if (peers_.count(peer.address()) == 0) {
          peers_[peer.address()] =
              network::createClient<proto::Yac>(peer.address());
        }
      }

    }  // namespace yac
  }    // namespace consensus
}  // namespace iroha
