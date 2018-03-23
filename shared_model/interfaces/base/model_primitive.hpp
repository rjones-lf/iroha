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

#ifndef IROHA_MODEL_PRIMITIVE_HPP
#define IROHA_MODEL_PRIMITIVE_HPP

#include "utils/string_builder.hpp"
#include "common/cloneable.hpp"

namespace shared_model {
  namespace interface {
    /**
     * ModelPrimitive is a base class of whole domain objects in system.
     * This class required for guarantee consistent interface on all shared
     * model objects.
     * @tparam Model - your new style model
     */
    template <typename Model>
    class ModelPrimitive : public Cloneable<ModelPrimitive<Model>> {
     public:
      /**
       * Reference for model type.
       */
      using ModelType = Model;

      /**
       * Make string developer representation of object
       * @return string with internal state of object
       */
      virtual std::string toString() const {
        return detail::PrettyStringBuilder()
            .init("Primitive")
            .append("address", std::to_string(reinterpret_cast<uint64_t>(this)))
            .finalize();
      }

      virtual bool operator==(const ModelType &rhs) const = 0;

      virtual bool operator!=(const ModelType &rhs) const {
        return not(*this == rhs);
      }

      virtual ~ModelPrimitive() = default;
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_MODEL_PRIMITIVE_HPP
