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

#ifndef IROHA_POSTGRES_WSV_COMMON_HPP
#define IROHA_POSTGRES_WSV_COMMON_HPP

#include <boost/optional.hpp>

#define SOCI_USE_BOOST
#define HAVE_BOOST
#include <soci/boost-tuple.h>
#include <soci/soci.h>

#include "common/result.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    /**
     * Transforms soci::rowset<soci::row> to vector of Ts by applying
     * transform_func
     * @tparam T - type to transform to
     * @tparam Operator - type of transformation function, must return T
     * @param result - soci::rowset<soci::row> which contains several rows from
     * the database
     * @param transform_func - function which transforms result row to T
     * @return vector of target type
     */
    template <typename T, typename Operator>
    std::vector<T> transform(const soci::rowset<soci::row> &result,
                             Operator &&transform_func) noexcept {
      std::vector<T> values;
      std::transform(result.begin(),
                     result.end(),
                     std::back_inserter(values),
                     transform_func);

      return values;
    }

    template <typename ParamType, typename Function>
    void processSoci(soci::statement &st,
                     soci::indicator &ind,
                     ParamType &row,
                     Function f) {
      while (st.fetch()) {
        switch (ind) {
          case soci::i_ok:
            f(row);
          case soci::i_null:
          case soci::i_truncated:
            break;
        }
      }
    }

    /**
     * Transforms result to optional
     * value -> optional<value>
     * error -> nullopt
     * @tparam T type of object inside
     * @param result BuilderResult
     * @return optional<T>
     */
    template <typename T>
    inline boost::optional<std::shared_ptr<T>> fromResult(
        shared_model::interface::CommonObjectsFactory::FactoryResult<
            std::unique_ptr<T>> &&result) {
      return result.match(
          [](expected::Value<std::unique_ptr<T>> &v) {
            return boost::make_optional(std::shared_ptr<T>(std::move(v.value)));
          },
          [](expected::Error<std::string>)
              -> boost::optional<std::shared_ptr<T>> { return boost::none; });
    }
  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_POSTGRES_WSV_COMMON_HPP
