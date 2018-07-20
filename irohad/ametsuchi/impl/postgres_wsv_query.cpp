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

#include "ametsuchi/impl/postgres_wsv_query.hpp"

#include <soci/boost-tuple.h>

#include "ametsuchi/impl/soci_utils.hpp"
#include "backend/protobuf/permissions.hpp"
#include "common/result.hpp"

namespace {
  /**
   * Transforms result to optional
   * value -> optional<value>
   * error -> nullopt
   * @tparam T type of object inside
   * @param result BuilderResult
   * @return optional<T>
   */
  template <typename T>
  boost::optional<std::shared_ptr<T>> fromResult(
      shared_model::interface::CommonObjectsFactory::FactoryResult<
          std::unique_ptr<T>> &&result) {
    return result.match(
        [](iroha::expected::Value<std::unique_ptr<T>> &v) {
          return boost::make_optional(std::shared_ptr<T>(std::move(v.value)));
        },
        [](iroha::expected::Error<std::string>)
            -> boost::optional<std::shared_ptr<T>> { return boost::none; });
  }
}  // namespace

namespace iroha {
  namespace ametsuchi {

    using shared_model::interface::types::AccountDetailKeyType;
    using shared_model::interface::types::AccountIdType;
    using shared_model::interface::types::AssetIdType;
    using shared_model::interface::types::DomainIdType;
    using shared_model::interface::types::JsonType;
    using shared_model::interface::types::PubkeyType;
    using shared_model::interface::types::RoleIdType;

    const std::string kRoleId = "role_id";
    const char *kAccountNotFound = "Account {} not found";
    const std::string kPublicKey = "public_key";
    const std::string kAssetId = "asset_id";
    const std::string kAccountId = "account_id";
    const std::string kDomainId = "domain_id";

    PostgresWsvQuery::PostgresWsvQuery(
        soci::session &sql,
        std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory)
        : sql_(sql), factory_(factory), log_(logger::log("PostgresWsvQuery")) {}

    PostgresWsvQuery::PostgresWsvQuery(
        std::unique_ptr<soci::session> sql_ptr,
        std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory)
        : sql_ptr_(std::move(sql_ptr)),
          sql_(*sql_ptr_),
          factory_(factory),
          log_(logger::log("PostgresWsvQuery")) {}

    bool PostgresWsvQuery::hasAccountGrantablePermission(
        const AccountIdType &permitee_account_id,
        const AccountIdType &account_id,
        shared_model::interface::permissions::Grantable permission) {
      const auto perm_str =
          shared_model::interface::GrantablePermissionSet({permission})
              .toBitstring();
      int size;
      soci::statement st = sql_.prepare
          << "SELECT count(*) FROM account_has_grantable_permissions WHERE "
             "permittee_account_id = :permittee_account_id AND account_id = "
             ":account_id "
             " AND permission & :permission = :permission ";

      st.exchange(soci::into(size));
      st.exchange(soci::use(permitee_account_id, "permittee_account_id"));
      st.exchange(soci::use(account_id, "account_id"));
      st.exchange(soci::use(perm_str, "permission"));
      st.define_and_bind();
      st.execute(true);

      return size == 1;
    }

    boost::optional<std::vector<RoleIdType>> PostgresWsvQuery::getAccountRoles(
        const AccountIdType &account_id) {
      std::vector<RoleIdType> roles;
      soci::indicator ind;
      std::string row;
      soci::statement st =
          (sql_.prepare << "SELECT role_id FROM account_has_roles WHERE "
                           "account_id = :account_id",
           soci::into(row, ind),
           soci::use(account_id));
      st.execute();

      processSoci(
          st, ind, row, [&roles](std::string &row) { roles.push_back(row); });
      return roles;
    }

    boost::optional<shared_model::interface::RolePermissionSet>
    PostgresWsvQuery::getRolePermissions(const RoleIdType &role_name) {
      shared_model::interface::RolePermissionSet set;
      soci::indicator ind;
      std::string row;
      soci::statement st =
          (sql_.prepare << "SELECT permission FROM role_has_permissions WHERE "
                           "role_id = :role_name",
           soci::into(row, ind),
           soci::use(role_name));
      st.execute();

      processSoci(st, ind, row, [&set](std::string &row) {
        set = shared_model::interface::RolePermissionSet(row);
      });
      return set;
    }

    boost::optional<std::vector<RoleIdType>> PostgresWsvQuery::getRoles() {
      soci::rowset<RoleIdType> roles =
          (sql_.prepare << "SELECT role_id FROM role");
      std::vector<RoleIdType> result;
      for (const auto &role : roles) {
        result.push_back(role);
      }
      return boost::make_optional(result);
    }

    boost::optional<std::shared_ptr<shared_model::interface::Account>>
    PostgresWsvQuery::getAccount(const AccountIdType &account_id) {
      boost::optional<std::string> domain_id, data;
      boost::optional<uint32_t> quorum;
      soci::statement st = sql_.prepare
          << "SELECT domain_id, quorum, data FROM account WHERE account_id = "
             ":account_id";

      st.exchange(soci::into(domain_id));
      st.exchange(soci::into(quorum));
      st.exchange(soci::into(data));
      st.exchange(soci::use(account_id, "account_id"));

      st.define_and_bind();
      st.execute(true);

      if (not domain_id) {
        return boost::none;
      }

      return fromResult(factory_->createAccount(
          account_id, domain_id.get(), quorum.get(), data.get()));
    }

    boost::optional<std::string> PostgresWsvQuery::getAccountDetail(
        const std::string &account_id,
        const AccountDetailKeyType &key,
        const AccountIdType &writer) {
      boost::optional<std::string> detail;

      if (key.empty() and writer.empty()) {
        // retrieve all values for a specified account
        std::string empty_json = "{}";
        sql_ << "SELECT data#>>:empty_json FROM account WHERE account_id = "
                ":account_id;",
            soci::into(detail), soci::use(empty_json), soci::use(account_id);
      } else if (not key.empty() and not writer.empty()) {
        // retrieve values for the account, under the key and added by the
        // writer
        std::string filled_json = "{\"" + writer + "\"" + ", \"" + key + "\"}";
        sql_ << "SELECT json_build_object(:writer::text, "
                "json_build_object(:key::text, (SELECT data #>> :filled_json "
                "FROM account WHERE account_id = :account_id)));",
            soci::into(detail), soci::use(writer), soci::use(key),
            soci::use(filled_json), soci::use(account_id);
      } else if (not writer.empty()) {
        // retrieve values added by the writer under all keys
        sql_ << "SELECT json_build_object(:writer::text, (SELECT data -> "
                ":writer FROM account WHERE account_id = :account_id));",
            soci::into(detail), soci::use(writer, "writer"),
            soci::use(account_id, "account_id");
      } else {
        // retrieve values from all writers under the key
        sql_ << "SELECT json_object_agg(key, value) AS json FROM (SELECT "
                "json_build_object(kv.key, json_build_object(:key::text, "
                "kv.value -> :key)) FROM jsonb_each((SELECT data FROM account "
                "WHERE account_id = :account_id)) kv WHERE kv.value ? :key) AS "
                "jsons, json_each(json_build_object);",
            soci::into(detail), soci::use(key, "key"),
            soci::use(account_id, "account_id");
      }

      return detail | [](auto &val) -> boost::optional<std::string> {
        // if val is empty, then there is no data for this account
        if (not val.empty()) {
          return val;
        }
        return boost::none;
      };
    }

    boost::optional<std::vector<PubkeyType>> PostgresWsvQuery::getSignatories(
        const AccountIdType &account_id) {
      std::vector<PubkeyType> pubkeys;
      soci::indicator ind;
      std::string row;
      soci::statement st =
          (sql_.prepare << "SELECT public_key FROM account_has_signatory WHERE "
                           "account_id = :account_id",
           soci::into(row, ind),
           soci::use(account_id));
      st.execute();

      processSoci(st, ind, row, [&pubkeys](std::string &row) {
        pubkeys.push_back(shared_model::crypto::PublicKey(
            shared_model::crypto::Blob::fromHexString(row)));
      });
      return boost::make_optional(pubkeys);
    }

    boost::optional<std::shared_ptr<shared_model::interface::Asset>>
    PostgresWsvQuery::getAsset(const AssetIdType &asset_id) {
      boost::optional<std::string> domain_id, data;
      boost::optional<int32_t> precision;
      soci::statement st = sql_.prepare
          << "SELECT domain_id, precision FROM asset WHERE asset_id = "
             ":account_id";
      st.exchange(soci::into(domain_id));
      st.exchange(soci::into(precision));
      st.exchange(soci::use(asset_id));

      st.define_and_bind();
      st.execute(true);

      if (not domain_id) {
        return boost::none;
      }

      return fromResult(
          factory_->createAsset(asset_id, domain_id.get(), precision.get()));
    }

    boost::optional<
        std::vector<std::shared_ptr<shared_model::interface::AccountAsset>>>
    PostgresWsvQuery::getAccountAssets(const AccountIdType &account_id) {
      using T = boost::tuple<std::string, std::string, std::string>;
      soci::rowset<T> st = (sql_.prepare << "SELECT * FROM account_has_asset "
                                            "WHERE account_id = :account_id",
                            soci::use(account_id));
      std::vector<std::shared_ptr<shared_model::interface::AccountAsset>>
          assets;
      for (auto &t : st) {
        fromResult(factory_->createAccountAsset(
            account_id,
            t.get<1>(),
            shared_model::interface::Amount(t.get<2>())))
            | [&assets](const auto &asset) { assets.push_back(asset); };
      }

      return boost::make_optional(assets);
    }

    boost::optional<std::shared_ptr<shared_model::interface::AccountAsset>>
    PostgresWsvQuery::getAccountAsset(const AccountIdType &account_id,
                                      const AssetIdType &asset_id) {
      boost::optional<std::string> amount;
      soci::statement st = sql_.prepare
          << "SELECT amount FROM account_has_asset WHERE account_id = "
             ":account_id AND asset_id = :asset_id";
      st.exchange(soci::into(amount));
      st.exchange(soci::use(account_id));
      st.exchange(soci::use(asset_id));
      st.define_and_bind();
      st.execute(true);

      if (not amount) {
        return boost::none;
      }

      return fromResult(factory_->createAccountAsset(
          account_id, asset_id, shared_model::interface::Amount(amount.get())));
    }

    boost::optional<std::shared_ptr<shared_model::interface::Domain>>
    PostgresWsvQuery::getDomain(const DomainIdType &domain_id) {
      boost::optional<std::string> role;
      soci::statement st = sql_.prepare
          << "SELECT default_role FROM domain WHERE domain_id = :id LIMIT 1";
      st.exchange(soci::into(role));
      st.exchange(soci::use(domain_id));
      st.define_and_bind();
      st.execute(true);

      if (not role) {
        return boost::none;
      }

      return fromResult(factory_->createDomain(domain_id, role.get()));
    }

    boost::optional<std::vector<std::shared_ptr<shared_model::interface::Peer>>>
    PostgresWsvQuery::getPeers() {
      soci::rowset<soci::row> rows =
          (sql_.prepare << "SELECT public_key, address FROM peer");
      std::vector<std::shared_ptr<shared_model::interface::Peer>> peers;

      for (auto &row : rows) {
        auto address = row.get<std::string>(1);
        auto key = shared_model::crypto::PublicKey(
            shared_model::crypto::Blob::fromHexString(row.get<std::string>(0)));

        auto peer = factory_->createPeer(address, key);
        peer.match(
            [&](expected::Value<std::unique_ptr<shared_model::interface::Peer>>
                    &v) { peers.push_back(std::move(v.value)); },
            [&](expected::Error<std::string> &e) { log_->info(e.error); });
      }
      return peers;
    }
  }  // namespace ametsuchi
}  // namespace iroha
