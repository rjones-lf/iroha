/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "ametsuchi/impl/mutable_storage_impl.hpp"

#include <boost/variant/apply_visitor.hpp>
#include "ametsuchi/impl/peer_query_wsv.hpp"
#include "ametsuchi/impl/postgres_block_index.hpp"
#include "ametsuchi/impl/postgres_command_executor.hpp"
#include "ametsuchi/impl/postgres_wsv_command.hpp"
#include "ametsuchi/impl/postgres_wsv_query.hpp"
#include "interfaces/commands/command.hpp"
#include "interfaces/common_objects/common_objects_factory.hpp"

namespace iroha {
  namespace ametsuchi {
    MutableStorageImpl::MutableStorageImpl(
        shared_model::interface::types::HashType top_hash,
        std::shared_ptr<PostgresCommandExecutor> cmd_executor,
        std::unique_ptr<soci::session> sql,
        std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory)
        : top_hash_(top_hash),
          sql_(std::move(sql)),
          peer_query_(std::make_unique<PeerQueryWsv>(
              std::make_shared<PostgresWsvQuery>(*sql_, std::move(factory)))),
          block_index_(std::make_unique<PostgresBlockIndex>(*sql_)),
          command_executor_(std::move(cmd_executor)),
          committed(false),
          log_(logger::log("MutableStorage")) {
      *sql_ << "BEGIN";
    }

    bool MutableStorageImpl::apply(const shared_model::interface::Block &block,
                                   MutableStoragePredicate predicate) {
      auto execute_transaction = [this](auto &transaction) {
        command_executor_->setCreatorAccountId(transaction.creatorAccountId());
        command_executor_->doValidation(false);

        auto execute_command = [this](const auto &command) {
          auto command_applied =
              boost::apply_visitor(*command_executor_, command.get());

          return command_applied.match(
              [](expected::Value<void> &) { return true; },
              [&](expected::Error<CommandError> &e) {
                log_->error(e.error.toString());
                return false;
              });
        };

        return std::all_of(transaction.commands().begin(),
                           transaction.commands().end(),
                           execute_command);
      };

      log_->info("Applying block: height {}, hash {}",
                 block.height(),
                 block.hash().hex());

      auto block_applied = predicate(block, *peer_query_, top_hash_)
          and std::all_of(block.transactions().begin(),
                          block.transactions().end(),
                          execute_transaction);
      if (block_applied) {
        block_store_.insert(std::make_pair(block.height(), clone(block)));
        block_index_->index(block);

        top_hash_ = block.hash();
      }

      return block_applied;
    }

    template <typename Function>
    bool MutableStorageImpl::withSavepoint(Function &&function) {
      *sql_ << "SAVEPOINT savepoint_";

      auto function_executed = std::forward<Function>(function)();

      if (function_executed) {
        *sql_ << "RELEASE SAVEPOINT savepoint_";
      } else {
        *sql_ << "ROLLBACK TO SAVEPOINT savepoint_";
      }

      return function_executed;
    }

    bool MutableStorageImpl::apply(
        const shared_model::interface::Block &block) {
      return withSavepoint([&] {
        return this->apply(
            block, [](const auto &, auto &, const auto &) { return true; });
      });
    }

    bool MutableStorageImpl::apply(
        rxcpp::observable<std::shared_ptr<shared_model::interface::Block>>
            blocks,
        MutableStoragePredicate predicate) {
      return withSavepoint([&] {
        return blocks
            .all([&](auto block) { return this->apply(*block, predicate); })
            .as_blocking()
            .first();
      });
    }

    MutableStorageImpl::~MutableStorageImpl() {
      if (not committed) {
        *sql_ << "ROLLBACK";
      }
    }
  }  // namespace ametsuchi
}  // namespace iroha
