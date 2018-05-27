/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_PROTO_PERMISSIONS_HPP
#define IROHA_SHARED_MODEL_PROTO_PERMISSIONS_HPP

#include "interfaces/permissions.hpp"

#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include "primitive.pb.h"
#include "utils/variant_deserializer.hpp"

namespace shared_model {
  namespace proto {
    namespace permissions {

      /**
       * @param perm protocol object for conversion
       * @return sm object if conversion can be done
       */
      boost::optional<interface::permissions::Role> fromTransport(
          iroha::protocol::RolePermission perm);
      /**
       * @param sm object for conversion
       * @return protobuf object
       */
      iroha::protocol::RolePermission toTransport(
          interface::permissions::Role r);
      /**
       * @param sm object for conversion
       * @return its string representation
       */
      std::string toString(interface::permissions::Role r);

      /**
       * @param perm protocol object for conversion
       * @return sm object if conversion can be done
       */
      boost::optional<interface::permissions::Grantable> fromTransport(
          iroha::protocol::GrantablePermission perm);
      /**
       * @param sm object for conversion
       * @return protobuf object
       */
      iroha::protocol::GrantablePermission toTransport(
          interface::permissions::Grantable r);
      /**
       * @param sm object for conversion
       * @return its string representation
       */
      std::string toString(interface::permissions::Role r);
    }  // namespace permissions
  }    // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_PERMISSIONS_HPP
