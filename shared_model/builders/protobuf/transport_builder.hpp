/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TRANSPORT_BUILDER_HPP
#define IROHA_TRANSPORT_BUILDER_HPP

#include "common/result.hpp"

namespace shared_model {
  namespace proto {

    /**
     * Class for building any shared model objects from corresponding transport
     * representation (e.g. protobuf object)
     * @tparam T Build type
     * @tparam SV Stateless validator type
     */
    template <typename T, typename SV>
    class [[deprecated]] TransportBuilder {
     public:
      TransportBuilder(const SV &validator = SV())
          : stateless_validator_(validator) {}

      /**
       * Builds result from transport object
       * @return value if transport object is valid and error message otherwise
       */
      iroha::expected::Result<T, std::string> build(
          typename T::TransportType transport) {
        auto result = T(transport);
        auto answer = stateless_validator_.validate(result);
        if (answer.hasErrors()) {
          return iroha::expected::makeError(answer.reason());
        }
        return iroha::expected::makeValue(T(std::move(transport)));
      }

     private:
      SV stateless_validator_;
    };

  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_TRANSPORT_BUILDER_HPP
