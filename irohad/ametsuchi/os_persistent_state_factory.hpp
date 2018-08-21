/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_OS_PERSISTENT_STATE_FACTORY_HPP
#define IROHA_OS_PERSISTENT_STATE_FACTORY_HPP

#include <memory>

#include "ametsuchi/ordering_service_persistent_state.hpp"

namespace iroha {
  namespace ametsuchi {
    class OsPersistentStateFactory {
     public:
      /**
       * @return ordering service persistent state
       */
      virtual boost::optional<std::shared_ptr<OrderingServicePersistentState>>
      createOsPersistentState() const = 0;

      virtual ~OsPersistentStateFactory() = default;
    };
  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_OS_PERSISTENT_STATE_FACTORY_HPP
