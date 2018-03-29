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

#include "builders/default_builders.hpp"
#include "common/result.hpp"
#include "logger/logger.hpp"

namespace iroha {
  namespace ametsuchi {

    /**
     * Return function which can execute SQL statements on provided transaction
     * @param transaction on which to apply statement.
     * @param logger is used to report an error.
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
     * @return boost::optional with pqxx::result in successful case, or nullopt
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

    /**
     * Execute build function and return error in case it throws
     * @tparam T - result value type
     * @param f - function which returns BuilderResult
     * @return whatever f returns, or error in case exception has been thrown
     */
    template <typename BuildFunc>
    static inline auto tryBuild(BuildFunc &&f) noexcept -> decltype(f()) {
      try {
        return f();
      } catch (std::exception &e) {
        return expected::makeError(std::make_shared<std::string>(e.what()));
      }
    }

    static inline shared_model::builder::BuilderResult<
        shared_model::interface::Account>
    makeAccount(const pqxx::row &row) noexcept {
      return tryBuild([&row] {
        return shared_model::builder::DefaultAccountBuilder()
            .accountId(row.at("account_id").template as<std::string>())
            .domainId(row.at("domain_id").template as<std::string>())
            .quorum(
                row.at("quorum")
                    .template as<shared_model::interface::types::QuorumType>())
            .jsonData(row.at("data").template as<std::string>())
            .build();
      });
    }

    static inline shared_model::builder::BuilderResult<
        shared_model::interface::Asset>
    makeAsset(const pqxx::row &row) noexcept {
      return tryBuild([&row] {
        return shared_model::builder::DefaultAssetBuilder()
            .assetId(row.at("asset_id").template as<std::string>())
            .domainId(row.at("domain_id").template as<std::string>())
            .precision(row.at("precision").template as<int32_t>())
            .build();
      });
    }

    static inline shared_model::builder::BuilderResult<
        shared_model::interface::AccountAsset>
    makeAccountAsset(const pqxx::row &row) noexcept {
      return tryBuild([&row] {
        auto balance = shared_model::builder::DefaultAmountBuilder::fromString(
            row.at("amount").template as<std::string>());
        return balance | [&](const auto &balance_ptr) {
          return shared_model::builder::DefaultAccountAssetBuilder()
              .accountId(row.at("account_id").template as<std::string>())
              .assetId(row.at("asset_id").template as<std::string>())
              .balance(*balance_ptr)
              .build();
        };
      });
    }

    static inline shared_model::builder::BuilderResult<
        shared_model::interface::Peer>
    makePeer(const pqxx::row &row) noexcept {
      return tryBuild([&row] {
        pqxx::binarystring public_key_str(row.at("public_key"));
        shared_model::interface::types::PubkeyType pubkey(public_key_str.str());
        return shared_model::builder::DefaultPeerBuilder()
            .pubkey(pubkey)
            .address(row.at("address").template as<std::string>())
            .build();
      });
    }

    static inline shared_model::builder::BuilderResult<
        shared_model::interface::Domain>
    makeDomain(const pqxx::row &row) noexcept {
      return tryBuild([&row] {
        return shared_model::builder::DefaultDomainBuilder()
            .domainId(row.at("domain_id").template as<std::string>())
            .defaultRole(row.at("default_role").template as<std::string>())
            .build();
      });
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
    static inline boost::optional<std::shared_ptr<T>> fromResult(
        const shared_model::builder::BuilderResult<T> &result) {
      return result.match(
          [](const expected::Value<std::shared_ptr<T>> &v) {
            return boost::make_optional(v.value);
          },
          [](const expected::Error<std::shared_ptr<std::string>> &e)
              -> boost::optional<std::shared_ptr<T>> {
            return boost::none;
          });
    }
  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_POSTGRES_WSV_COMMON_HPP
