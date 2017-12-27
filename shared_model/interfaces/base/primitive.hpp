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

#ifndef IROHA_SHARED_MODEL_PRIMITIVE_HPP
#define IROHA_SHARED_MODEL_PRIMITIVE_HPP

#include "interfaces/base/model_primitive.hpp"
#include "utils/swig_keyword_hider.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Primitive is the base class for all model objects, that require backward
     * compatibility
     * @tparam Model - your new style model;
     * @tparam OldModel - old-style model, that changed with new model;
     */
    template <typename Model, typename OldModel>
    class Primitive : public ModelPrimitive<Model> {
     public:
      /**
       * Reference for old-style model type
       */
      using OldModelType = OldModel;

      /**
       * Create new old-style object for model.
       * Useful for backward compatibility.
       * You should to avoid this method and write code with new model.
       * This method should be removed when all components reworked with new
       * model
       * @return pointer for old-style object
       */
      DEPRECATED virtual OldModelType *makeOldModel() const = 0;
    };

  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_PRIMITIVE_HPP
