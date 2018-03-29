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

#ifndef IROHA_ORDERING_SERVICE_PERSISTENT_STATE_HPP
#define IROHA_ORDERING_SERVICE_PERSISTENT_STATE_HPP

#include <boost/optional.hpp>

namespace iroha {
  namespace ametsuchi {

    /**
     * Interface for Ordering Service persistence to store proposal's height in
     * a persistent way
     */
    class OrderingServicePersistentState {
     public:
      /**
       * Save proposal height that it can be restored
       * after launch
       */
      virtual bool saveProposalHeight(size_t height) = 0;

      /**
       * Load proposal height
       */
      virtual boost::optional<size_t> loadProposalHeight() const = 0;

      /**
       * Reset storage to default state
       */
      virtual bool resetState() = 0;
    };
  }  // namespace ametsuchi
}  // namespace iroha

#endif  // IROHA_ORDERING_SERVICE_PERSISTENT_STATE_HPP
