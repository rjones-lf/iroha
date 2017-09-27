/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_MST_STATE_HPP
#define IROHA_MST_STATE_HPP

#include <unordered_set>
#include <memory>
#include "model/transaction.hpp"

namespace iroha {
  class MstState {
   public:

    /**
     * Concat internal data of states
     * @param rhs - object for joining
     * @return State is union of this and right states
     */
    virtual MstState operator+(const MstState &rhs);

    /**
     * Operator provide difference between this and rhs operator
     * @param rhs, state for removing
     * @return State that provide difference between left and right states
     * axiom operators:
     * A + B == B + A
     * A - B == A + (B - A)
     */
    virtual MstState operator-(const MstState &rhs);

    /**
     * Provide transactions, that contains in state
     */
    virtual std::unordered_set<std::shared_ptr<model::Transaction>> getTransactions();

    virtual ~MstState() = default;
  };
} // namespace iroha

#endif //IROHA_MST_STATE_HPP
