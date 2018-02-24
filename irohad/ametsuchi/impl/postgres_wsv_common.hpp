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
#include <pqxx/nontransaction>
#include <pqxx/result>
#include "common/result.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    /**
     * Return function which can execute SQL statements on provided transaction
     * @param transaction on which to apply statement.
     * @return Result with pqxx::result in value case, or exception message
     * if exception was caught
     */
    inline auto makeExecuteResult(pqxx::nontransaction &transaction) noexcept {
      return [&](const std::string &statement) noexcept
          ->expected::Result<pqxx::result, std::string> {
        try {
          return expected::makeValue(transaction.exec(statement));
        } catch (const std::exception &e) {
          return expected::makeError(e.what());
        }
      };
    }

    /**
     * Return function which can execute SQL statements on provided transaction
     * This function is deprecated, and will be removed as soon as wsv_query
     * will be refactored to return result
     * @param transaction on which to apply statement.
     * @param logger is used to report an error.
     * @return nonstd::optional with pqxx::result in successful case, or nullopt
     * if exception was caught
     */
    inline auto makeExecuteOptional(pqxx::nontransaction &transaction,
                                    logger::Logger &logger) noexcept {
      return [&](const std::string &statement) noexcept
          ->boost::optional<pqxx::result> {
        try {
          return transaction.exec(statement);
        } catch (const std::exception &e) {
          logger->error(e.what());
          return boost::none;
        }
      };
    }

    /**
     * Transforms pqxx::result to vector of Ts by applying transform_func
     * @tparam T - type to transform to
     * @tparam Operator - type of transformation function, must return T
     * @param result - pqxx::result which contains several rows from the
     * database
     * @param transform_func - function which transforms result row to T
     * @return vector of target type
     */
    template <typename T, typename Operator>
    std::vector<T> transform(const pqxx::result &result,
                             Operator &&transform_func) noexcept {
      std::vector<T> values;
      values.reserve(result.size());
      std::transform(result.begin(),
                     result.end(),
                     std::back_inserter(values),
                     transform_func);

      return values;
    }
  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_POSTGRES_WSV_COMMON_HPP
