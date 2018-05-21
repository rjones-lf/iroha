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

#ifndef IROHA_EMPTY_BLOCK_VALIDATOR_HPP
#define IROHA_EMPTY_BLOCK_VALIDATOR_HPP

#include "interfaces/iroha_internal/empty_block.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    template <typename FieldValidator>
    class EmptyBlockValidator {
     public:
      EmptyBlockValidator(FieldValidator field_validator = FieldValidator())
          : field_validator_(field_validator) {}
      /**
       * Applies validation on block
       * @param block
       * @return Answer containing found error if any
       */
      Answer validate(const interface::EmptyBlock &block) const {
        Answer answer;
        ReasonsGroupType reason;
        reason.first = "EmptyBlock";
        field_validator_.validateCreatedTime(reason, block.createdTime());
        field_validator_.validateHeight(reason, block.height());
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }
        return answer;
      }

     private:
      FieldValidator field_validator_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_BLOCK_VALIDATOR_HPP
