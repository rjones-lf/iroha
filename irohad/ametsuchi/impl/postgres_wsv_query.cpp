/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_wsv_query.hpp"

#include <soci/boost-tuple.h>
#include "ametsuchi/impl/soci_utils.hpp"
#include "backend/protobuf/permissions.hpp"
#include "common/result.hpp"
#include "cryptography/public_key.hpp"

namespace iroha {
  namespace ametsuchi {

    using shared_model::interface::types::AccountDetailKeyType;
    using shared_model::interface::types::AccountIdType;
    using shared_model::interface::types::AddressType;
    using shared_model::interface::types::AssetIdType;
    using shared_model::interface::types::DetailType;
    using shared_model::interface::types::DomainIdType;
    using shared_model::interface::types::JsonType;
    using shared_model::interface::types::PrecisionType;
    using shared_model::interface::types::PubkeyType;
    using shared_model::interface::types::QuorumType;
    using shared_model::interface::types::RoleIdType;

    PostgresWsvQuery::PostgresWsvQuery(
        soci::session &sql,
        std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory)
        : sql_(sql), factory_(factory), log_(logger::log("PostgresWsvQuery")) {}

    PostgresWsvQuery::PostgresWsvQuery(
        std::unique_ptr<soci::session> sql,
        std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory)
        : psql_(std::move(sql)),
          sql_(*psql_),
          factory_(factory),
          log_(logger::log("PostgresWsvQuery")) {}

    template <typename T>
    boost::optional<std::shared_ptr<T>> PostgresWsvQuery::fromResult(
        shared_model::interface::CommonObjectsFactory::FactoryResult<
            std::unique_ptr<T>> &&result) {
      return result.match(
          [](iroha::expected::Value<std::unique_ptr<T>> &v) {
            return boost::make_optional(std::shared_ptr<T>(std::move(v.value)));
          },
          [&](iroha::expected::Error<std::string> &e)
              -> boost::optional<std::shared_ptr<T>> {
            log_->error(e.error);
            return boost::none;
          });
    }

    template <typename T, typename F>
    auto PostgresWsvQuery::execute(F &&f) -> boost::optional<soci::rowset<T>> {
      try {
        return soci::rowset<T>{std::forward<F>(f)()};
      } catch (const std::exception &e) {
        log_->error("Failed to execute query: {}", e.what());
        return boost::none;
      }
    }

    bool PostgresWsvQuery::hasAccountGrantablePermission(
        const AccountIdType &permitee_account_id,
        const AccountIdType &account_id,
        shared_model::interface::permissions::Grantable permission) {
      const auto perm_str =
          shared_model::interface::GrantablePermissionSet({permission})
              .toBitstring();
      using T = boost::tuple<int>;
      auto result = execute<T>([&] {
        return (sql_.prepare
                    << "SELECT count(*) FROM "
                       "account_has_grantable_permissions WHERE "
                       "permittee_account_id = :permittee_account_id AND "
                       "account_id = "
                       ":account_id "
                       " AND permission & :permission = :permission ",
                soci::use(permitee_account_id, "permittee_account_id"),
                soci::use(account_id, "account_id"),
                soci::use(perm_str, "permission"));
      });

      return flatMapValue<boost::optional<bool>>(
                 result,
                 [](auto &count) { return boost::make_optional(count == 1); })
          .value_or(false);
    }

    boost::optional<std::vector<RoleIdType>> PostgresWsvQuery::getAccountRoles(
        const AccountIdType &account_id) {
      using T = boost::tuple<RoleIdType>;
      auto result = execute<T>([&] {
        return (sql_.prepare << "SELECT role_id FROM account_has_roles WHERE "
                                "account_id = :account_id",
                soci::use(account_id));
      });

      return mapValues<std::vector<RoleIdType>>(
          result, [&](auto &role_id) { return role_id; });
    }

    boost::optional<shared_model::interface::RolePermissionSet>
    PostgresWsvQuery::getRolePermissions(const RoleIdType &role_name) {
      using T = boost::tuple<std::string>;
      auto result = execute<T>([&] {
        return (sql_.prepare
                    << "SELECT permission FROM role_has_permissions WHERE "
                       "role_id = :role_name",
                soci::use(role_name));
      });

      return result | [&](auto &&st)
                 -> boost::optional<
                     shared_model::interface::RolePermissionSet> {
        auto range = boost::make_iterator_range(st);

        if (range.empty()) {
          return shared_model::interface::RolePermissionSet{};
        }

        return apply(range.front(), [](auto &permission) {
          return shared_model::interface::RolePermissionSet(permission);
        });
      };
    }

    boost::optional<std::vector<RoleIdType>> PostgresWsvQuery::getRoles() {
      using T = boost::tuple<RoleIdType>;
      auto result = execute<T>(
          [&] { return (sql_.prepare << "SELECT role_id FROM role"); });

      return mapValues<std::vector<RoleIdType>>(
          result, [&](auto &role_id) { return role_id; });
    }

    boost::optional<std::shared_ptr<shared_model::interface::Account>>
    PostgresWsvQuery::getAccount(const AccountIdType &account_id) {
      using T = boost::tuple<DomainIdType, QuorumType, JsonType>;
      auto result = execute<T>([&] {
        return (sql_.prepare << "SELECT domain_id, quorum, data "
                                "FROM account WHERE account_id = "
                                ":account_id",
                soci::use(account_id, "account_id"));
      });

      return flatMapValue<
          boost::optional<std::shared_ptr<shared_model::interface::Account>>>(
          result, [&](auto &domain_id, auto quorum, auto &data) {
            return this->fromResult(
                factory_->createAccount(account_id, domain_id, quorum, data));
          });
    }

    boost::optional<std::string> PostgresWsvQuery::getAccountDetail(
        const std::string &account_id,
        const AccountDetailKeyType &key,
        const AccountIdType &writer) {
      using T = boost::tuple<DetailType>;
      boost::optional<soci::rowset<T>> result;

      if (key.empty() and writer.empty()) {
        // retrieve all values for a specified account
        std::string empty_json = "{}";
        result = execute<T>([&] {
          return (sql_.prepare
                      << "SELECT data#>>:empty_json FROM account WHERE "
                         "account_id = "
                         ":account_id;",
                  soci::use(empty_json),
                  soci::use(account_id));
        });
      } else if (not key.empty() and not writer.empty()) {
        // retrieve values for the account, under the key and added by the
        // writer
        std::string filled_json = "{\"" + writer + "\"" + ", \"" + key + "\"}";
        result = execute<T>([&] {
          return (sql_.prepare
                      << "SELECT json_build_object(:writer::text, "
                         "json_build_object(:key::text, (SELECT data #>> "
                         ":filled_json "
                         "FROM account WHERE account_id = :account_id)));",
                  soci::use(writer),
                  soci::use(key),
                  soci::use(filled_json),
                  soci::use(account_id));
        });
      } else if (not writer.empty()) {
        // retrieve values added by the writer under all keys
        result = execute<T>([&] {
          return (
              sql_.prepare
                  << "SELECT json_build_object(:writer::text, (SELECT data -> "
                     ":writer FROM account WHERE account_id = :account_id));",
              soci::use(writer, "writer"),
              soci::use(account_id, "account_id"));
        });
      } else {
        // retrieve values from all writers under the key
        result = execute<T>([&] {
          return (
              sql_.prepare
                  << "SELECT json_object_agg(key, value) AS json FROM (SELECT "
                     "json_build_object(kv.key, json_build_object(:key::text, "
                     "kv.value -> :key)) FROM jsonb_each((SELECT data FROM "
                     "account "
                     "WHERE account_id = :account_id)) kv WHERE kv.value ? "
                     ":key) "
                     "AS "
                     "jsons, json_each(json_build_object);",
              soci::use(key, "key"),
              soci::use(account_id, "account_id"));
        });
      }

      return flatMapValue<boost::optional<std::string>>(
          result, [&](auto &json) { return boost::make_optional(json); });
    }

    boost::optional<std::vector<PubkeyType>> PostgresWsvQuery::getSignatories(
        const AccountIdType &account_id) {
      using T = boost::tuple<std::string>;
      auto result = execute<T>([&] {
        return (sql_.prepare
                    << "SELECT public_key FROM account_has_signatory WHERE "
                       "account_id = :account_id",
                soci::use(account_id));
      });

      return mapValues<std::vector<PubkeyType>>(result, [&](auto &public_key) {
        return shared_model::crypto::PublicKey{
            shared_model::crypto::Blob::fromHexString(public_key)};
      });
    }

    boost::optional<std::shared_ptr<shared_model::interface::Asset>>
    PostgresWsvQuery::getAsset(const AssetIdType &asset_id) {
      using T = boost::tuple<DomainIdType, PrecisionType>;
      auto result = execute<T>([&] {
        return (
            sql_.prepare
                << "SELECT domain_id, precision FROM asset WHERE asset_id = "
                   ":asset_id",
            soci::use(asset_id));
      });

      return flatMapValue<
          boost::optional<std::shared_ptr<shared_model::interface::Asset>>>(
          result, [&](auto &domain_id, auto precision) {
            return this->fromResult(
                factory_->createAsset(asset_id, domain_id, precision));
          });
    }

    boost::optional<
        std::vector<std::shared_ptr<shared_model::interface::AccountAsset>>>
    PostgresWsvQuery::getAccountAssets(const AccountIdType &account_id) {
      using T = boost::tuple<AssetIdType, std::string>;
      auto result = execute<T>([&] {
        return (sql_.prepare
                    << "SELECT asset_id, amount FROM account_has_asset "
                       "WHERE account_id = :account_id",
                soci::use(account_id));
      });

      return flatMapValues<
          std::vector<std::shared_ptr<shared_model::interface::AccountAsset>>>(
          result, [&](auto &asset_id, auto &amount) {
            return this->fromResult(factory_->createAccountAsset(
                account_id, asset_id, shared_model::interface::Amount(amount)));
          });
    }

    boost::optional<std::shared_ptr<shared_model::interface::AccountAsset>>
    PostgresWsvQuery::getAccountAsset(const AccountIdType &account_id,
                                      const AssetIdType &asset_id) {
      using T = boost::tuple<std::string>;
      auto result = execute<T>([&] {
        return (
            sql_.prepare
                << "SELECT amount FROM account_has_asset WHERE account_id = "
                   ":account_id AND asset_id = :asset_id",
            soci::use(account_id),
            soci::use(asset_id));
      });

      return flatMapValue<boost::optional<
          std::shared_ptr<shared_model::interface::AccountAsset>>>(
          result, [&](auto &amount) {
            return this->fromResult(factory_->createAccountAsset(
                account_id, asset_id, shared_model::interface::Amount(amount)));
          });
    }

    boost::optional<std::shared_ptr<shared_model::interface::Domain>>
    PostgresWsvQuery::getDomain(const DomainIdType &domain_id) {
      using T = boost::tuple<RoleIdType>;
      auto result = execute<T>([&] {
        return (sql_.prepare << "SELECT default_role FROM domain "
                                "WHERE domain_id = :id LIMIT 1",
                soci::use(domain_id));
      });

      return flatMapValue<
          boost::optional<std::shared_ptr<shared_model::interface::Domain>>>(
          result, [&](auto &default_role) {
            return this->fromResult(
                factory_->createDomain(domain_id, default_role));
          });
    }

    boost::optional<std::vector<std::shared_ptr<shared_model::interface::Peer>>>
    PostgresWsvQuery::getPeers() {
      using T = boost::tuple<std::string, AddressType>;
      auto result = execute<T>([&] {
        return (sql_.prepare << "SELECT public_key, address FROM peer");
      });

      return flatMapValues<
          std::vector<std::shared_ptr<shared_model::interface::Peer>>>(
          result, [&](auto &public_key, auto &address) {
            return this->fromResult(factory_->createPeer(
                address,
                shared_model::crypto::PublicKey{
                    shared_model::crypto::Blob::fromHexString(public_key)}));
          });
    }
  }  // namespace ametsuchi
}  // namespace iroha
