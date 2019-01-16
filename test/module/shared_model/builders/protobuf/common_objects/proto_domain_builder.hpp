/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_DOMAIN_BUILDER_HPP
#define IROHA_PROTO_DOMAIN_BUILDER_HPP

#include "backend/protobuf/common_objects/domain.hpp"
#include "qry_responses.pb.h"

namespace shared_model {
  namespace proto {
    class DEPRECATED DomainBuilder {
     public:
      shared_model::proto::Domain build() {
        return shared_model::proto::Domain(iroha::protocol::Domain(domain_));
      }

      DomainBuilder defaultRole(
          const interface::types::RoleIdType &default_role) {
        DomainBuilder copy(*this);
        copy.domain_.set_default_role(default_role);
        return copy;
      }

      DomainBuilder domainId(const interface::types::DomainIdType &domain_id) {
        DomainBuilder copy(*this);
        copy.domain_.set_domain_id(domain_id);
        return copy;
      }

     private:
      iroha::protocol::Domain domain_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_DOMAIN_BUILDER_HPP
