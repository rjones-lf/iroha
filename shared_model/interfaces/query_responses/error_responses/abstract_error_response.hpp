
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

#ifndef IROHA_ABSTRACT_ERROR_RESPONSE_HPP
#define IROHA_ABSTRACT_ERROR_RESPONSE_HPP

#include "interfaces/base/primitive.hpp"
#include "utils/string_builder.hpp"

#ifndef DISABLE_BACKWARD
#include "model/queries/responses/error_response.hpp"
#endif

namespace shared_model {
  namespace interface {
    /**
     * Abstract error response
     * @tparam Model - concrete model error response
     */
    template <typename Model>
    class AbstractErrorResponse
        : public PRIMITIVE_WITH_OLD(Model, iroha::model::ErrorResponse) {
     private:
      /**
       * @return string representation of error reason
       */
      virtual std::string reason() const = 0;

#ifndef DISABLE_BACKWARD
      /**
       * @return old model error reason
       */
      virtual iroha::model::ErrorResponse::Reason oldModelReason() const = 0;

#endif
     public:
      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return detail::PrettyStringBuilder().init(reason()).finalize();
      }

#ifndef DISABLE_BACKWARD
      iroha::model::ErrorResponse *makeOldModel() const override {
        auto error_response = new iroha::model::ErrorResponse();
        error_response->reason = oldModelReason();
        return error_response;
      }

#endif

      bool operator==(const Model &rhs) const override {
        return true;
      }
    };
  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_ABSTRACT_ERROR_RESPONSE_HPP
