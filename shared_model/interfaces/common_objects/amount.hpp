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

#ifndef IROHA_SHARED_MODEL_AMOUNT_HPP
#define IROHA_SHARED_MODEL_AMOUNT_HPP

#include <boost/multiprecision/cpp_dec_float.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <vector>
#include "amount/amount.hpp"
#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    /**
     * Representation of fixed point number
     */
    class Amount : public PRIMITIVE_WITH_OLD(Amount, iroha::Amount) {
     public:
      /**
       * Gets integer representation value, which ignores precision
       * @return amount represented as integer value, which ignores precision
       */
      virtual const boost::multiprecision::uint256_t &intValue() const = 0;

      /**
       * Gets the position of precision
       * @return the position of precision
       */
      virtual types::PrecisionType precision() const = 0;

      /**
       * Checks equality of objects inside
       * @param rhs - other wrapped value
       * @return true, if wrapped objects are same
       */
      bool operator==(const ModelType &rhs) const override {
        return intValue() == rhs.intValue() and precision() == rhs.precision();
      }

      /**
       * String representation.
       * @return string representation of the asset.
       */
      std::string toStringRepr() const {
        if (precision() > 0) {
          boost::multiprecision::cpp_dec_float_50 float50(intValue());
          float50 /= pow(10, precision());
          return float50.str(precision(), std::ios_base::fixed);
        }
        return intValue().str(0, std::ios_base::fixed);
      }

      /**
       * Stringify the data.
       * @return the content of asset.
       */
      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Amount")
            .append("intValue", intValue().str())
            .append("precision", std::to_string(precision()))
            .finalize();
      }

#ifndef DISABLE_BACKWARD
      /**
       * Makes old model.
       * @return An allocated old model of account asset response.
       */
      OldModelType *makeOldModel() const override {
        return new OldModelType(intValue(), precision());
      }

#endif
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_AMOUNT_HPP
