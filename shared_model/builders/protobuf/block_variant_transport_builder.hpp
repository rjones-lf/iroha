/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BLOCK_VARIANT_TRANSPORT_BUILDER_HPP
#define IROHA_BLOCK_VARIANT_TRANSPORT_BUILDER_HPP

#include "builders/protobuf/transport_builder.hpp"
#include "interfaces/iroha_internal/block_variant.hpp"

namespace shared_model {
  namespace proto {

    /**
     * Class for building BlockVariantType containing either Block or EmptyBlock
     * @tparam SV Stateless validator type
     */
    template <typename SV>
    class TransportBuilder<interface::BlockVariantType, SV> {
     private:
      /**
       * Create container type (i.e. Block or EmptyBlock)
       * @tparam T container type
       * @param transport is protobuf object from which container type is built
       * @return Result containing BlockVariantType or error message with string
       * type
       */
      template <typename T>
      iroha::expected::Result<interface::BlockVariantType, std::string>
      createContainer(const iroha::protocol::Block &transport) {
        auto result = std::make_shared<T>(transport);
        auto answer = stateless_validator_.validate(*result);
        if (answer.hasErrors()) {
          return iroha::expected::makeError(answer.reason());
        }
        return iroha::expected::makeValue(result);
      }

     public:
      TransportBuilder<interface::BlockVariantType, SV>(
          SV stateless_validator = SV())
          : stateless_validator_(stateless_validator) {}

      /**
       * Builds BlockVariantType from transport object
       * @param transport -- protobuf object from which BlockVariant is built
       * @return Result containing either BlockVariantType or message string
       */
      iroha::expected::Result<interface::BlockVariantType, std::string> build(
          iroha::protocol::Block transport) {
        if (transport.payload().transactions().size() == 0) {
          return createContainer<shared_model::proto::EmptyBlock>(transport);
        }
        return createContainer<shared_model::proto::Block>(transport);
      }

     private:
      SV stateless_validator_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_BLOCK_VARIANT_TRANSPORT_BUILDER_HPP
