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

#ifndef IROHA_ABSTRACT_TX_RESPONSE_HPP
#define IROHA_ABSTRACT_TX_RESPONSE_HPP

#include "interfaces/base/model_primitive.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {
    /**
     * Abstract transaction response
     * @tparam Model - concrete model transaction response
     */
    template <typename Model>
    class AbstractTxResponse : public ModelPrimitive<Model> {
     private:
      /**
       * @return string representation of class name
       */
      virtual std::string className() const = 0;

     public:
      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return detail::PrettyStringBuilder().init(className()).finalize();
      }

      bool operator==(const Model &rhs) const override {
        return true;
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_ABSTRACT_TX_RESPONSE_HPP
