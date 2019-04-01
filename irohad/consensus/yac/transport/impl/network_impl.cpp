/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/transport/impl/network_impl.hpp"

#include <grpc++/grpc++.h>
#include <memory>

#include "consensus/yac/storage/yac_common.hpp"
#include "consensus/yac/transport/yac_pb_converters.hpp"
#include "consensus/yac/vote_message.hpp"
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
              async_call,
          logger::LoggerPtr log)
          : async_call_(async_call), log_(std::move(log)) {}

      void NetworkImpl::subscribe(
          std::shared_ptr<YacNetworkNotifications> handler) {
        handler_ = handler;
      }

      YacNetworkWithFeedBack::SendStateReturnType
      NetworkImpl::sendState(const shared_model::interface::Peer &to,
                             const std::vector<VoteMessage> &state) {
        createPeerConnection(to);

        proto::State request;
        for (const auto &vote : state) {
          auto pb_vote = request.add_votes();
          *pb_vote = PbConverters::serializeVote(vote);
        }

        log_->info(
            "Send votes bundle[size={}] to {}", state.size(), to.address());

        auto log_outcome = [log = log_, destination_peer = to.toString()](
                               const grpc::Status &status) {
          log->info("Sent to {} with status details [{}]",
                    destination_peer,
                    status.ok() ? "OK" : status.error_details());
        };

        return async_call_
            ->Call([&](auto context, auto cq) {
              return peers_.at(to.address())
                  ->AsyncSendState(context, request, cq);
            })
            .tap(log_outcome)
            .map(
                [](const auto &status) { return makeSendStateStatus(status); });
      }

      YacNetworkWithFeedBack::ValueStateReturnType
      NetworkImpl::makeSendStateStatus(const grpc::Status &status) {
        auto is_ok = [](const auto &code) {
          return code == grpc::StatusCode::OK;
        };

        auto is_troubles_with_recipient = [](const auto &code) {
          using namespace grpc;
          std::vector<int> codes = {StatusCode::CANCELLED,
                                    StatusCode::INVALID_ARGUMENT,
                                    StatusCode::UNAUTHENTICATED,
                                    StatusCode::RESOURCE_EXHAUSTED,
                                    StatusCode::ABORTED,
                                    StatusCode::UNIMPLEMENTED,
                                    StatusCode::UNAVAILABLE,
                                    StatusCode::DATA_LOSS};
          return std::any_of(codes.begin(), codes.end(), [code](auto val) {
            return code == val;
          });
        };

        auto code = status.error_code();

        using namespace iroha::consensus::yac::sending_statuses;

        if (is_ok(code)) {
          return SuccessfulSent();
        }

        if (is_troubles_with_recipient(code)) {
          return UnavailableReceiver();
        }

        return UnavailableNetwork();
      }

      grpc::Status NetworkImpl::SendState(
          ::grpc::ServerContext *context,
          const ::iroha::consensus::yac::proto::State *request,
          ::google::protobuf::Empty *response) {
        std::vector<VoteMessage> state;
        for (const auto &pb_vote : request->votes()) {
          auto vote = *PbConverters::deserializeVote(pb_vote, log_);
          state.push_back(vote);
        }
        if (state.empty()) {
          log_->info("Received an empty votes collection");
          return grpc::Status::CANCELLED;
        }
        if (not sameKeys(state)) {
          log_->info(
              "Votes are statelessly invalid: proposal rounds are different");
          return grpc::Status::CANCELLED;
        }

        log_->info(
            "Received votes[size={}] from {}", state.size(), context->peer());

        if (auto notifications = handler_.lock()) {
          notifications->onState(std::move(state));
        } else {
          log_->error("Unable to lock the subscriber");
        }
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
