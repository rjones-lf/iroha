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

#include "builders/default_builders.hpp"
#include "common/result.hpp"
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

    static inline shared_model::builder::BuilderResult<
        shared_model::interface::Account>
    makeAccount(const std::string &account_id,
                const std::string &domain_id,
                const shared_model::interface::types::QuorumType &quorum,
                const std::string &data) noexcept {
      return tryBuild([&] {
        return shared_model::builder::DefaultAccountBuilder()
            .accountId(account_id)
            .domainId(domain_id)
            .quorum(quorum)
            .jsonData(data)
            .build();
      });
    }

    static inline shared_model::builder::BuilderResult<
        shared_model::interface::Asset>
    makeAsset(const std::string &asset_id,
              const std::string &domain_id,
              const int32_t precision) noexcept {
      return tryBuild([&] {
        return shared_model::builder::DefaultAssetBuilder()
            .assetId(asset_id)
            .domainId(domain_id)
            .precision(precision)
            .build();
      });
    }

    static inline shared_model::builder::BuilderResult<
        shared_model::interface::AccountAsset>
    makeAccountAsset(const std::string &account_id,
                     const std::string &asset_id,
                     const std::string &amount) noexcept {
      return tryBuild([&] {
        return shared_model::builder::DefaultAccountAssetBuilder()
            .accountId(account_id)
            .assetId(asset_id)
            .balance(shared_model::interface::Amount(amount))
            .build();
      });
    }

    static inline shared_model::builder::BuilderResult<
        shared_model::interface::Peer>
    makePeer(const soci::row &row) noexcept {
      return tryBuild([&row] {
        return shared_model::builder::DefaultPeerBuilder()
            .pubkey(shared_model::crypto::PublicKey(
                shared_model::crypto::Blob::fromHexString(
                    row.get<std::string>(0))))
            .address(row.get<std::string>(1))
            .build();
      });
    }

    static inline shared_model::builder::BuilderResult<
        shared_model::interface::Domain>
    makeDomain(const std::string &domain_id, const std::string &role) noexcept {
      return tryBuild([&domain_id, &role] {
        return shared_model::builder::DefaultDomainBuilder()
            .domainId(domain_id)
            .defaultRole(role)
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
              -> boost::optional<std::shared_ptr<T>> { return boost::none; });
    }
  }  // namespace ametsuchi
}  // namespace iroha
#endif  // IROHA_POSTGRES_WSV_COMMON_HPP
