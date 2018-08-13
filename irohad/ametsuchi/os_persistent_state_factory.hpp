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
    class OSPersistentStateFactory {
     public:
      /**
       * @return ordering service persistent state
       */
      virtual boost::optional<std::shared_ptr<OrderingServicePersistentState>>
      createOSPersistentState() const = 0;

      virtual ~OSPersistentStateFactory() = default;
    };
  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_OS_PERSISTENT_STATE_FACTORY_HPP
