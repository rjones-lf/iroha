/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/postgres_query_executor.hpp"

#include <boost-tuple.h>
#include <soci/boost-tuple.h>
#include <soci/postgresql/soci-postgresql.h>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/irange.hpp>
#include "ametsuchi/impl/soci_utils.hpp"
#include "common/types.hpp"
#include "interfaces/queries/blocks_query.hpp"

using namespace shared_model::interface::permissions;

namespace {

  using namespace iroha;

  /**
   * A query response that contains an error response
   * @tparam T error type
   */
  template <typename T>
  auto error_response = shared_model::proto::TemplateQueryResponseBuilder<>()
                            .errorQueryResponse<T>();

  /**
   * A query response that contains StatefulFailed error
   */
  auto stateful_failed =
      error_response<shared_model::interface::StatefulFailedErrorResponse>;

  std::string getDomainFromName(const std::string &account_id) {
    std::vector<std::string> res;
    boost::split(res, account_id, boost::is_any_of("@"));
    return res.at(1);
  }

  std::string checkAccountRolePermission(
      shared_model::interface::permissions::Role permission,
      const std::string &account_alias = "role_account_id") {
    const auto perm_str =
        shared_model::interface::RolePermissionSet({permission}).toBitstring();
    const auto bits = shared_model::interface::RolePermissionSet::size();
    // 14.09.18 andrei: IR-1708 Load SQL from separate files
    std::string query = (boost::format(R"(
          SELECT (COALESCE(bit_or(rp.permission), '0'::bit(%1%))
          & '%2%') = '%2%' AS perm FROM role_has_permissions AS rp
              JOIN account_has_roles AS ar on ar.role_id = rp.role_id
              WHERE ar.account_id = :%3%)")
                         % bits % perm_str % account_alias)
                            .str();
    return query;
  }

  /**
   * Generate an SQL subquery which checks if creator has corresponding
   * permissions for target account
   * It verifies individual, domain, and global permissions, and returns true if
   * any of listed permissions is present
   */
  auto hasQueryPermission(const std::string &creator,
                          const std::string &target_account,
                          Role indiv_permission_id,
                          Role all_permission_id,
                          Role domain_permission_id) {
    const auto bits = shared_model::interface::RolePermissionSet::size();
    const auto perm_str =
        shared_model::interface::RolePermissionSet({indiv_permission_id})
            .toBitstring();
    const auto all_perm_str =
        shared_model::interface::RolePermissionSet({all_permission_id})
            .toBitstring();
    const auto domain_perm_str =
        shared_model::interface::RolePermissionSet({domain_permission_id})
            .toBitstring();

    boost::format cmd(R"(
    WITH
        has_indiv_perm AS (
          SELECT (COALESCE(bit_or(rp.permission), '0'::bit(%1%))
          & '%3%') = '%3%' FROM role_has_permissions AS rp
              JOIN account_has_roles AS ar on ar.role_id = rp.role_id
              WHERE ar.account_id = '%2%'
        ),
        has_all_perm AS (
          SELECT (COALESCE(bit_or(rp.permission), '0'::bit(%1%))
          & '%4%') = '%4%' FROM role_has_permissions AS rp
              JOIN account_has_roles AS ar on ar.role_id = rp.role_id
              WHERE ar.account_id = '%2%'
        ),
        has_domain_perm AS (
          SELECT (COALESCE(bit_or(rp.permission), '0'::bit(%1%))
          & '%5%') = '%5%' FROM role_has_permissions AS rp
              JOIN account_has_roles AS ar on ar.role_id = rp.role_id
              WHERE ar.account_id = '%2%'
        )
    SELECT ('%2%' = '%6%' AND (SELECT * FROM has_indiv_perm))
        OR (SELECT * FROM has_all_perm)
        OR ('%7%' = '%8%' AND (SELECT * FROM has_domain_perm)) AS perm
    )");

    return (cmd % bits % creator % perm_str % all_perm_str % domain_perm_str
            % target_account % getDomainFromName(creator)
            % getDomainFromName(target_account))
        .str();
  }

  /// Query result is a tuple of optionals, since there could be no entry
  template <typename... Value>
  using QueryType = boost::tuple<boost::optional<Value>...>;

  /// return statefulFailed if no permissions in given tuple are set, nullopt
  /// otherwise
  template <typename T>
  auto validatePermissions(const T &t) {
    return ametsuchi::apply(t, [](auto... perms) {
      bool temp[] = {not perms...};
      return boost::make_optional<ametsuchi::QueryResponseBuilderDone>(
          std::all_of(
              std::begin(temp), std::end(temp), [](auto b) { return b; }),
          stateful_failed);
    });
  }

}  // namespace

namespace iroha {
  namespace ametsuchi {

    template <typename RangeGen, typename Pred>
    auto PostgresQueryExecutorVisitor::getTransactionsFromBlock(
        uint64_t block_id, RangeGen &&range_gen, Pred &&pred) {
      std::vector<shared_model::proto::Transaction> result;
      auto serialized_block = block_store_.get(block_id);
      if (not serialized_block) {
        log_->error("Failed to retrieve block with id {}", block_id);
        return result;
      }
      auto deserialized_block =
          converter_->deserialize(bytesToString(*serialized_block));
      if (auto e =
              boost::get<expected::Error<std::string>>(&deserialized_block)) {
        log_->error(e->error);
        return result;
      }

      auto &block =
          boost::get<
              expected::Value<std::unique_ptr<shared_model::interface::Block>>>(
              deserialized_block)
              .value;

      boost::transform(
          range_gen(boost::size(block->transactions()))
              | boost::adaptors::transformed(
                    [&block](auto i) -> decltype(auto) {
                      return block->transactions()[i];
                    })
              | boost::adaptors::filtered(pred),
          std::back_inserter(result),
          [&](const auto &tx) {
            return *static_cast<shared_model::proto::Transaction *>(
                clone(tx).get());
          });

      return result;
    }

    using QueryResponseBuilder =
        shared_model::proto::TemplateQueryResponseBuilder<>;

    PostgresQueryExecutor::PostgresQueryExecutor(
        std::unique_ptr<soci::session> sql,
        std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory,
        KeyValueStorage &block_store,
        std::shared_ptr<PendingTransactionStorage> pending_txs_storage,
        std::shared_ptr<shared_model::interface::BlockJsonConverter> converter)
        : sql_(std::move(sql)),
          block_store_(block_store),
          factory_(factory),
          pending_txs_storage_(pending_txs_storage),
          visitor_(*sql_,
                   factory_,
                   block_store_,
                   pending_txs_storage_,
                   std::move(converter)) {}

    QueryExecutorResult PostgresQueryExecutor::validateAndExecute(
        const shared_model::interface::Query &query) {
      visitor_.setCreatorId(query.creatorAccountId());
      auto result = boost::apply_visitor(visitor_, query.get());
      return clone(result.queryHash(query.hash()).build());
    }

    bool PostgresQueryExecutor::validate(
        const shared_model::interface::BlocksQuery &query) {
      using T = boost::tuple<int>;
      boost::format cmd(R"(%s)");
      soci::rowset<T> st =
          (sql_->prepare
               << (cmd % checkAccountRolePermission(Role::kGetBlocks)).str(),
           soci::use(query.creatorAccountId(), "role_account_id"));

      return st.begin()->get<0>();
    }

    PostgresQueryExecutorVisitor::PostgresQueryExecutorVisitor(
        soci::session &sql,
        std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory,
        KeyValueStorage &block_store,
        std::shared_ptr<PendingTransactionStorage> pending_txs_storage,
        std::shared_ptr<shared_model::interface::BlockJsonConverter> converter)
        : sql_(sql),
          block_store_(block_store),
          factory_(factory),
          pending_txs_storage_(pending_txs_storage),
          converter_(std::move(converter)),
          log_(logger::log("PostgresQueryExecutorVisitor")) {}

    void PostgresQueryExecutorVisitor::setCreatorId(
        const shared_model::interface::types::AccountIdType &creator_id) {
      creator_id_ = creator_id;
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetAccount &q) {
      using Q = QueryType<shared_model::interface::types::AccountIdType,
                          shared_model::interface::types::DomainIdType,
                          shared_model::interface::types::QuorumType,
                          shared_model::interface::types::DetailType,
                          std::string>;
      using P = boost::tuple<int>;
      using T = concat<Q, P>;

      auto cmd = (boost::format(R"(WITH has_perms AS (%s),
      t AS (
          SELECT a.account_id, a.domain_id, a.quorum, a.data, ARRAY_AGG(ar.role_id) AS roles
          FROM account AS a, account_has_roles AS ar
          WHERE a.account_id = :target_account_id
          AND ar.account_id = a.account_id
          GROUP BY a.account_id
      )
      SELECT account_id, domain_id, quorum, data, roles, perm
      FROM t RIGHT OUTER JOIN has_perms AS p ON TRUE
      )")
                  % hasQueryPermission(creator_id_,
                                       q.accountId(),
                                       Role::kGetMyAccount,
                                       Role::kGetAllAccounts,
                                       Role::kGetDomainAccounts))
                     .str();

      soci::rowset<T> st =
          (sql_.prepare << cmd, soci::use(q.accountId(), "target_account_id"));
      auto &tuple = *st.begin();

      auto query_apply = [this](auto &account_id,
                                auto &domain_id,
                                auto &quorum,
                                auto &data,
                                auto &roles_str) {
        return factory_->createAccount(account_id, domain_id, quorum, data)
            .match(
                [roles_str =
                     roles_str.substr(1, roles_str.size() - 2)](auto &v) {
                  std::vector<shared_model::interface::types::RoleIdType> roles;

                  boost::split(
                      roles, roles_str, [](char c) { return c == ','; });

                  return QueryResponseBuilder().accountResponse(
                      *static_cast<shared_model::proto::Account *>(
                          v.value.get()),
                      roles);
                },
                [this](expected::Error<std::string> &e) {
                  log_->error(e.error);
                  return stateful_failed;
                });
      };

      return validatePermissions(viewRest<P>(tuple))
          .value_or_eval([this, &tuple, &query_apply] {
            return match_in_place(
                rebind(viewTuple<Q>(tuple)),
                [this, &query_apply](auto &&t) {
                  return apply(t, query_apply);
                },
                [] {
                  return error_response<
                      shared_model::interface::NoAccountErrorResponse>;
                });
          });
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetSignatories &q) {
      using Q = QueryType<std::string>;
      using P = boost::tuple<int>;
      using T = concat<Q, P>;

      auto cmd = (boost::format(R"(WITH has_perms AS (%s),
      t AS (
          SELECT public_key FROM account_has_signatory
          WHERE account_id = :account_id
      )
      SELECT public_key, perm FROM t
      RIGHT OUTER JOIN has_perms ON TRUE
      )")
                  % hasQueryPermission(creator_id_,
                                       q.accountId(),
                                       Role::kGetMySignatories,
                                       Role::kGetAllSignatories,
                                       Role::kGetDomainSignatories))
                     .str();

      soci::rowset<T> st = (sql_.prepare << cmd, soci::use(q.accountId()));
      // get iterators since they are single pass
      auto begin = st.begin(), end = st.end();

      return validatePermissions(viewRest<P>(*begin))
          .value_or_eval([&begin, &end] {
            std::vector<shared_model::interface::types::PubkeyType> pubkeys;
            std::for_each(begin, end, [&pubkeys](auto &t) {
              rebind(viewTuple<Q>(t)) | [&pubkeys](auto &&t) {
                apply(t, [&pubkeys](auto &public_key) {
                  pubkeys.emplace_back(
                      shared_model::crypto::Blob::fromHexString(public_key));
                });
              };
            });
            return boost::make_optional<QueryResponseBuilderDone>(
                       pubkeys.empty(),
                       error_response<
                           shared_model::interface::NoSignatoriesErrorResponse>)
                .value_or_eval([&pubkeys] {
                  return QueryResponseBuilder().signatoriesResponse(pubkeys);
                });
          });
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetAccountTransactions &q) {
      using Q = QueryType<shared_model::interface::types::HeightType, uint64_t>;
      using P = boost::tuple<int>;
      using T = concat<Q, P>;

      auto cmd = (boost::format(R"(WITH has_perms AS (%s),
      t AS (
          SELECT DISTINCT has.height, index
          FROM height_by_account_set AS has
          JOIN index_by_creator_height AS ich ON has.height = ich.height
          AND has.account_id = ich.creator_id
          WHERE account_id = :account_id
          ORDER BY has.height, index ASC
      )
      SELECT height, index, perm FROM t
      RIGHT OUTER JOIN has_perms ON TRUE
      )")
                  % hasQueryPermission(creator_id_,
                                       q.accountId(),
                                       Role::kGetMyAccTxs,
                                       Role::kGetAllAccTxs,
                                       Role::kGetDomainAccTxs))
                     .str();
      soci::rowset<T> st = (sql_.prepare << cmd, soci::use(q.accountId()));
      auto begin = st.begin(), end = st.end();

      auto deserialize = [this, &begin, &end] {
        std::map<uint64_t, std::vector<uint64_t>> index;
        std::for_each(begin, end, [&index](auto &t) {
          rebind(viewTuple<Q>(t)) | [&index](auto &&t) {
            apply(t, [&index](auto &height, auto &idx) {
              index[height].push_back(idx);
            });
          };
        });

        std::vector<shared_model::proto::Transaction> proto;
        for (auto &block : index) {
          auto txs =
              getTransactionsFromBlock(block.first,
                                       [&block](auto) { return block.second; },
                                       [](auto &) { return true; });
          std::move(txs.begin(), txs.end(), std::back_inserter(proto));
        }

        return QueryResponseBuilder().transactionsResponse(proto);
      };

      return validatePermissions(viewRest<P>(*begin))
          .value_or_eval(deserialize);
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetTransactions &q) {
      auto escape = [](auto &hash) { return "'" + hash.hex() + "'"; };
      std::string hash_str = std::accumulate(
          std::next(q.transactionHashes().begin()),
          q.transactionHashes().end(),
          escape(q.transactionHashes().front()),
          [&escape](auto &acc, auto &val) { return acc + "," + escape(val); });

      using Q =
          QueryType<shared_model::interface::types::HeightType, std::string>;
      using P = boost::tuple<int, int>;
      using T = concat<Q, P>;

      auto cmd = (boost::format(R"(WITH has_my_perm AS (%s),
      has_all_perm AS (%s),
      t AS (
          SELECT height, hash FROM height_by_hash WHERE hash IN (%s)
      )
      SELECT height, hash, has_my_perm.perm, has_all_perm.perm FROM t
      RIGHT OUTER JOIN has_my_perm ON TRUE
      RIGHT OUTER JOIN has_all_perm ON TRUE
      )") % checkAccountRolePermission(Role::kGetMyTxs, "account_id")
                  % checkAccountRolePermission(Role::kGetAllTxs, "account_id")
                  % hash_str)
                     .str();
      soci::rowset<T> st =
          (sql_.prepare << cmd, soci::use(creator_id_, "account_id"));
      auto begin = st.begin(), end = st.end();

      auto deserialize = [this, &begin, &end](auto &my_perm, auto &all_perm) {
        std::map<uint64_t, std::unordered_set<std::string>> index;
        std::for_each(begin, end, [&index](auto &t) {
          rebind(viewTuple<Q>(t)) | [&index](auto &&t) {
            apply(t, [&index](auto &height, auto &hash) {
              index[height].insert(hash);
            });
          };
        });

        std::vector<shared_model::proto::Transaction> proto;
        for (auto &block : index) {
          auto txs = this->getTransactionsFromBlock(
              block.first,
              [](auto size) {
                return boost::irange(static_cast<decltype(size)>(0), size);
              },
              [&](auto &tx) {
                return block.second.count(tx.hash().hex()) > 0
                    and (all_perm
                         or (my_perm and tx.creatorAccountId() == creator_id_));
              });
          std::move(txs.begin(), txs.end(), std::back_inserter(proto));
        }

        return QueryResponseBuilder().transactionsResponse(proto);
      };

      return validatePermissions(viewRest<P>(*begin))
          .value_or_eval([this, &begin, &end, &deserialize] {
            return apply(viewRest<P>(*begin), deserialize);
          });
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetAccountAssetTransactions &q) {
      using Q = QueryType<shared_model::interface::types::HeightType, uint64_t>;
      using P = boost::tuple<int>;
      using T = concat<Q, P>;

      auto cmd = (boost::format(R"(WITH has_perms AS (%s),
      t AS (
          SELECT DISTINCT has.height, index
          FROM height_by_account_set AS has
          JOIN index_by_id_height_asset AS ich ON has.height = ich.height
          AND has.account_id = ich.id
          WHERE account_id = :account_id
          AND asset_id = :asset_id
          ORDER BY has.height, index ASC
      )
      SELECT height, index, perm FROM t
      RIGHT OUTER JOIN has_perms ON TRUE
      )")
                  % hasQueryPermission(creator_id_,
                                       q.accountId(),
                                       Role::kGetMyAccAstTxs,
                                       Role::kGetAllAccAstTxs,
                                       Role::kGetDomainAccAstTxs))
                     .str();

      soci::rowset<T> st = (sql_.prepare << cmd,
                            soci::use(q.accountId(), "account_id"),
                            soci::use(q.assetId(), "asset_id"));
      auto begin = st.begin(), end = st.end();

      auto deserialize = [this, &begin, &end] {
        std::map<uint64_t, std::vector<uint64_t>> index;
        std::for_each(begin, end, [&index](auto &t) {
          rebind(viewTuple<Q>(t)) | [&index](auto &&t) {
            apply(t, [&index](auto &height, auto &idx) {
              index[height].push_back(idx);
            });
          };
        });

        std::vector<shared_model::proto::Transaction> proto;
        for (auto &block : index) {
          auto txs =
              getTransactionsFromBlock(block.first,
                                       [&block](auto) { return block.second; },
                                       [](auto &) { return true; });
          std::move(txs.begin(), txs.end(), std::back_inserter(proto));
        }

        return QueryResponseBuilder().transactionsResponse(proto);
      };

      return validatePermissions(viewRest<P>(*begin))
          .value_or_eval(deserialize);
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetAccountAssets &q) {
      using Q = QueryType<shared_model::interface::types::AccountIdType,
                          shared_model::interface::types::AssetIdType,
                          std::string>;
      using P = boost::tuple<int>;
      using T = concat<Q, P>;

      auto cmd = (boost::format(R"(WITH has_perms AS (%s),
      t AS (
          SELECT * FROM account_has_asset
          WHERE account_id = :account_id
      )
      SELECT account_id, asset_id, amount, perm FROM t
      RIGHT OUTER JOIN has_perms ON TRUE
      )")
                  % hasQueryPermission(creator_id_,
                                       q.accountId(),
                                       Role::kGetMyAccAst,
                                       Role::kGetAllAccAst,
                                       Role::kGetDomainAccAst))
                     .str();
      soci::rowset<T> st = (sql_.prepare << cmd, soci::use(q.accountId()));
      auto begin = st.begin(), end = st.end();

      return validatePermissions(viewRest<P>(*begin))
          .value_or_eval([this, &begin, &end] {
            std::vector<shared_model::proto::AccountAsset> account_assets;
            std::for_each(begin, end, [this, &account_assets](auto &t) {
              rebind(viewTuple<Q>(t)) | [this, &account_assets](auto &&t) {
                apply(t,
                      [this, &account_assets](
                          auto &account_id, auto &asset_id, auto &amount) {
                        factory_
                            ->createAccountAsset(
                                account_id,
                                asset_id,
                                shared_model::interface::Amount(amount))
                            .match(
                                [&account_assets](auto &v) {
                                  auto proto = *static_cast<
                                      shared_model::proto::AccountAsset *>(
                                      v.value.get());
                                  account_assets.push_back(proto);
                                },
                                [this](expected::Error<std::string> &e) {
                                  log_->error(e.error);
                                });
                      });
              };
            });
            return QueryResponseBuilder().accountAssetResponse(account_assets);
          });
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetAccountDetail &q) {
      using Q = QueryType<shared_model::interface::types::DetailType>;
      using P = boost::tuple<int>;
      using T = concat<Q, P>;

      std::string query_detail;
      if (q.key() and q.writer()) {
        auto filled_json = (boost::format("{\"%s\", \"%s\"}") % q.writer().get()
                            % q.key().get());
        query_detail = (boost::format(R"(SELECT json_build_object('%s'::text,
            json_build_object('%s'::text, (SELECT data #>> '%s'
            FROM account WHERE account_id = :account_id))) AS json)")
                        % q.writer().get() % q.key().get() % filled_json)
                           .str();
      } else if (q.key() and not q.writer()) {
        query_detail =
            (boost::format(
                 R"(SELECT json_object_agg(key, value) AS json FROM (SELECT
            json_build_object(kv.key, json_build_object('%1%'::text,
            kv.value -> '%1%')) FROM jsonb_each((SELECT data FROM account
            WHERE account_id = :account_id)) kv WHERE kv.value ? '%1%') AS
            jsons, json_each(json_build_object))")
             % q.key().get())
                .str();
      } else if (not q.key() and q.writer()) {
        query_detail = (boost::format(R"(SELECT json_build_object('%1%'::text,
          (SELECT data -> '%1%' FROM account WHERE account_id =
           :account_id)) AS json)")
                        % q.writer().get())
                           .str();
      } else {
        query_detail = (boost::format(R"(SELECT data#>>'{}' AS json FROM account
            WHERE account_id = :account_id)"))
                           .str();
      }
      auto cmd = (boost::format(R"(WITH has_perms AS (%s),
      detail AS (%s)
      SELECT json, perm FROM detail
      RIGHT OUTER JOIN has_perms ON TRUE
      )")
                  % hasQueryPermission(creator_id_,
                                       q.accountId(),
                                       Role::kGetMyAccDetail,
                                       Role::kGetAllAccDetail,
                                       Role::kGetDomainAccDetail)
                  % query_detail)
                     .str();
      soci::rowset<T> st =
          (sql_.prepare << cmd, soci::use(q.accountId(), "account_id"));
      auto &tuple = *st.begin();

      return validatePermissions(viewRest<P>(tuple))
          .value_or_eval([this, &tuple] {
            return match_in_place(
                rebind(viewTuple<Q>(tuple)),
                [this](auto &&t) {
                  return apply(t, [this](auto &json) {
                    return QueryResponseBuilder().accountDetailResponse(json);
                  });
                },
                [] {
                  return error_response<
                      shared_model::interface::NoAccountDetailErrorResponse>;
                });
          });
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetRoles &q) {
      using Q = QueryType<shared_model::interface::types::RoleIdType>;
      using P = boost::tuple<int>;
      using T = concat<Q, P>;

      auto cmd = (boost::format(
                      R"(WITH has_perms AS (%s)
      SELECT role_id, perm FROM role
      RIGHT OUTER JOIN has_perms ON TRUE
      )") % checkAccountRolePermission(Role::kGetRoles))
                     .str();
      soci::rowset<T> st =
          (sql_.prepare << cmd, soci::use(creator_id_, "role_account_id"));
      auto begin = st.begin(), end = st.end();

      return validatePermissions(viewRest<P>(*begin))
          .value_or_eval([&begin, &end] {
            std::vector<shared_model::interface::types::RoleIdType> roles;
            std::for_each(begin, end, [&roles](auto &t) {
              rebind(viewTuple<Q>(t)) | [&roles](auto &&t) {
                apply(t, [&roles](auto &role_id) { roles.push_back(role_id); });
              };
            });

            // roles vector is never empty, since an account is required to
            // perform a query, and therefore a domain

            return QueryResponseBuilder().rolesResponse(roles);
          });
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetRolePermissions &q) {
      using Q = QueryType<std::string>;
      using P = boost::tuple<int>;
      using T = concat<Q, P>;

      auto cmd = (boost::format(
                      R"(WITH has_perms AS (%s),
      perms AS (SELECT permission FROM role_has_permissions
                WHERE role_id = :role_name)
      SELECT permission, perm FROM perms
      RIGHT OUTER JOIN has_perms ON TRUE
      )") % checkAccountRolePermission(Role::kGetRoles))
                     .str();

      soci::rowset<T> st = (sql_.prepare << cmd,
                            soci::use(creator_id_, "role_account_id"),
                            soci::use(q.roleId(), "role_name"));
      auto &tuple = *st.begin();

      return validatePermissions(viewRest<P>(tuple))
          .value_or_eval([this, &tuple] {
            return match_in_place(
                rebind(viewTuple<Q>(tuple)),
                [this](auto &&t) {
                  return apply(t, [this](auto &permission) {
                    return QueryResponseBuilder().rolePermissionsResponse(
                        shared_model::interface::RolePermissionSet(permission));
                  });
                },
                [] {
                  return error_response<
                      shared_model::interface::NoRolesErrorResponse>;
                });
          });
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetAssetInfo &q) {
      using Q =
          QueryType<shared_model::interface::types::DomainIdType, uint32_t>;
      using P = boost::tuple<int>;
      using T = concat<Q, P>;

      auto cmd = (boost::format(
                      R"(WITH has_perms AS (%s),
      perms AS (SELECT domain_id, precision FROM asset
                WHERE asset_id = :asset_id)
      SELECT domain_id, precision, perm FROM perms
      RIGHT OUTER JOIN has_perms ON TRUE
      )") % checkAccountRolePermission(Role::kReadAssets))
                     .str();
      soci::rowset<T> st = (sql_.prepare << cmd,
                            soci::use(creator_id_, "role_account_id"),
                            soci::use(q.assetId(), "asset_id"));
      auto &tuple = *st.begin();

      return validatePermissions(viewRest<P>(tuple))
          .value_or_eval([this, &tuple, &q] {
            return match_in_place(
                rebind(viewTuple<Q>(tuple)),
                [this, &q](auto &&t) {
                  return apply(t, [this, &q](auto &domain_id, auto &precision) {
                    return QueryResponseBuilder().assetResponse(
                        q.assetId(), domain_id, precision);
                  });
                },
                [] {
                  return error_response<
                      shared_model::interface::NoAssetErrorResponse>;
                });
          });
    }

    QueryResponseBuilderDone PostgresQueryExecutorVisitor::operator()(
        const shared_model::interface::GetPendingTransactions &q) {
      std::vector<shared_model::proto::Transaction> txs;
      auto interface_txs =
          pending_txs_storage_->getPendingTransactions(creator_id_);
      txs.reserve(interface_txs.size());

      std::transform(
          interface_txs.begin(),
          interface_txs.end(),
          std::back_inserter(txs),
          [](auto &tx) {
            return *(
                std::static_pointer_cast<shared_model::proto::Transaction>(tx));
          });

      // TODO 2018-08-07, rework response builder - it should take
      // interface::Transaction, igor-egorov, IR-1041
      auto response = QueryResponseBuilder().transactionsResponse(txs);
      return response;
    }

  }  // namespace ametsuchi
}  // namespace iroha
