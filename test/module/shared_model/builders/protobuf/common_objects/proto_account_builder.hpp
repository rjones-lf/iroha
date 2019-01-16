/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_ACCOUNT_BUILDER_HPP
#define IROHA_PROTO_ACCOUNT_BUILDER_HPP

#include "backend/protobuf/common_objects/account.hpp"
#include "qry_responses.pb.h"

namespace shared_model {
  namespace proto {

    /**
     * AccountBuilder is used to construct Account proto objects with
     * initialized protobuf implementation
     */
    class DEPRECATED AccountBuilder {
     public:
      shared_model::proto::Account build() {
        return shared_model::proto::Account(iroha::protocol::Account(account_));
      }

      AccountBuilder accountId(
          const interface::types::AccountIdType &account_id) {
        AccountBuilder copy(*this);
        copy.account_.set_account_id(account_id);
        return copy;
      }

      AccountBuilder domainId(const interface::types::DomainIdType &domain_id) {
        AccountBuilder copy(*this);
        copy.account_.set_domain_id(domain_id);
        return copy;
      }

      AccountBuilder quorum(const interface::types::QuorumType &quorum) {
        AccountBuilder copy(*this);
        copy.account_.set_quorum(quorum);
        return copy;
      }

      AccountBuilder jsonData(const interface::types::JsonType &json_data) {
        AccountBuilder copy(*this);
        copy.account_.set_json_data(json_data);
        return copy;
      }

     private:
      iroha::protocol::Account account_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_ACCOUNT_BUILDER_HPP
