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
      Answer validateProtoQuery(const iroha::protocol::Query &qry) const {
        Answer answer;
        if (qry.payload().query_case()
            == iroha::protocol::Query_Payload::QUERY_NOT_SET) {
          ReasonsGroupType reason;
          reason.first = "Undefined";
          reason.second.emplace_back("query is undefined");
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
