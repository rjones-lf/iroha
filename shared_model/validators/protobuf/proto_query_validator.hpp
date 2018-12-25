/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_QUERY_VALIDATOR_HPP
#define IROHA_PROTO_QUERY_VALIDATOR_HPP

#include "validators/abstract_validator.hpp"

#include "queries.pb.h"

namespace shared_model {
  namespace validation {

    class ProtoQueryValidator
        : public AbstractValidator<iroha::protocol::Query> {
     private:
      void validatePaginationQuery(
          const iroha::protocol::TxPaginationMeta &paginationMeta,
          ReasonsGroupType &reason) const {
        if (paginationMeta.opt_first_tx_hash_case()
            != iroha::protocol::TxPaginationMeta::OPT_FIRST_TX_HASH_NOT_SET) {
          if (not validateHexString(paginationMeta.first_tx_hash())) {
            reason.second.emplace_back("First tx hash has invalid format");
          }
        }
      }

      Answer validateProtoQuery(const iroha::protocol::Query &qry) const {
        Answer answer;
        std::string tx_reason_name = "Protobuf Query";
        ReasonsGroupType reason(tx_reason_name, GroupedReasons());
        switch (qry.payload().query_case()) {
          case iroha::protocol::Query_Payload::QUERY_NOT_SET: {
            reason.second.emplace_back("query is undefined");
            break;
          }
          case iroha::protocol::Query_Payload::kGetAccountTransactions: {
            const auto &gat = qry.payload().get_account_transactions();
            validatePaginationQuery(gat.pagination_meta(), reason);
            break;
          }
          case iroha::protocol::Query_Payload::kGetAccountAssetTransactions: {
            const auto &gat = qry.payload().get_account_asset_transactions();
            validatePaginationQuery(gat.pagination_meta(), reason);
            break;
          }
          default: { break; }
        }
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }
        return answer;
      }

     public:
      Answer validate(const iroha::protocol::Query &query) const override {
        return validateProtoQuery(query);
      }
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_PROTO_QUERY_VALIDATOR_HPP
