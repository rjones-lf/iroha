/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#ifndef SHARED_MODEL_FROM_OLD_HPP
#define SHARED_MODEL_FROM_OLD_HPP
#ifndef DISABLE_BACKWARD

#include <boost/range/adaptor/transformed.hpp>

#include "backend/protobuf/block.hpp"
#include "backend/protobuf/proposal.hpp"
#include "backend/protobuf/queries/proto_query.hpp"
#include "backend/protobuf/query_responses/proto_query_response.hpp"
#include "backend/protobuf/transaction.hpp"
#include "builders/protobuf/proposal.hpp"
#include "model/converters/pb_block_factory.hpp"
#include "model/converters/pb_command_factory.hpp"
#include "model/converters/pb_query_factory.hpp"
#include "model/converters/pb_query_response_factory.hpp"
#include "model/converters/pb_transaction_factory.hpp"

#include "backend/protobuf/common_objects/account.hpp"
#include "backend/protobuf/common_objects/account_asset.hpp"
#include "backend/protobuf/common_objects/asset.hpp"
#include "backend/protobuf/common_objects/domain.hpp"
#include "backend/protobuf/common_objects/peer.hpp"
#include "model/converters/pb_common.hpp"

namespace shared_model {
  namespace proto {

    inline static shared_model::proto::Block from_old(
        const iroha::model::Block &block) {
      return shared_model::proto::Block(
          iroha::model::converters::PbBlockFactory().serialize(block));
    }

    inline static shared_model::proto::Transaction from_old(
        const iroha::model::Transaction &tx) {
      return shared_model::proto::Transaction(
          iroha::model::converters::PbTransactionFactory().serialize(tx));
    }

    inline static shared_model::proto::Query from_old(
        std::shared_ptr<const iroha::model::Query> qry) {
      return shared_model::proto::Query(
          *iroha::model::converters::PbQueryFactory().serialize(qry));
    }

    inline static shared_model::proto::Proposal from_old(
        const iroha::model::Proposal &proposal) {
      return shared_model::proto::ProposalBuilder()
          .height(proposal.height)
          .createdTime(proposal.created_time)
          .transactions(proposal.transactions
                        | boost::adaptors::transformed(
                              [](auto &tx) { return from_old(tx); }))
          .build();
    }

    inline static shared_model::proto::QueryResponse from_old(
        std::shared_ptr<iroha::model::QueryResponse> queryResponse) {
      auto proto_resp =
          *iroha::model::converters::PbQueryResponseFactory().serialize(
              queryResponse);
      auto res = shared_model::proto::QueryResponse(std::move(proto_resp));
      auto hash = res.queryHash();
      return shared_model::proto::QueryResponse(
          *iroha::model::converters::PbQueryResponseFactory().serialize(
              queryResponse));
    }

    inline static shared_model::proto::Account from_old(
        const iroha::model::Account &account) {
      return shared_model::proto::Account(
          iroha::model::converters::serializeAccount(account));
    }

    inline static shared_model::proto::Peer from_old(
        const iroha::model::Peer &peer) {
      return shared_model::proto::Peer(
          iroha::model::converters::serializePeer(peer));
    }

    inline static shared_model::proto::Asset from_old(
        const iroha::model::Asset &asset) {
      return shared_model::proto::Asset(
          iroha::model::converters::serializeAsset(asset));
    }

    inline static shared_model::proto::AccountAsset from_old(
        const iroha::model::AccountAsset &account_asset) {
      return shared_model::proto::AccountAsset(
          iroha::model::converters::serializeAccountAsset(account_asset));
    }

    inline static shared_model::proto::Domain from_old(
        const iroha::model::Domain &domain) {
      return shared_model::proto::Domain(
          iroha::model::converters::serializeDomain(domain));
    }

    inline static shared_model::proto::Amount from_old(
        const iroha::Amount &amount) {
      return shared_model::proto::Amount(
          iroha::model::converters::serializeAmount(amount));
    }

    inline static shared_model::proto::Command from_old(
        const iroha::model::Command &command) {
      return shared_model::proto::Command(
          iroha::model::converters::PbCommandFactory().serializeAbstractCommand(
              command));
    }

  }  // namespace proto
}  // namespace shared_model

#endif  // DISABLE_BACKWARD
#endif  // SHARED_MODEL_FROM_OLD_HPP
