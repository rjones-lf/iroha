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
#include "model/sha3_hash.hpp"

namespace iroha {
  namespace ametsuchi {
    MutableStorageImpl::MutableStorageImpl(
        shared_model::interface::types::HashType top_hash,
        std::unique_ptr<soci::session> sql,
        std::shared_ptr<shared_model::interface::CommonObjectsFactory> factory)
        : top_hash_(top_hash),
          sql_(std::move(sql)),
          wsv_(std::make_shared<PostgresWsvQuery>(*sql_, factory)),
          block_index_(std::make_unique<PostgresBlockIndex>(*sql_)),
          command_executor_(std::make_shared<PostgresCommandExecutor>(*sql_)),
          committed(false),
          log_(logger::log("MutableStorage")) {
      *sql_ << "BEGIN";
    }

    bool MutableStorageImpl::check(
        const shared_model::interface::Block &block,
        MutableStoragePredicate predicate) {
      PeerQueryWsv peer_query(wsv_);
      return predicate(block, peer_query, top_hash_);
    }

    bool MutableStorageImpl::apply(const shared_model::interface::Block &block) {
      auto execute_transaction = [this](auto &transaction) {
        command_executor_->setCreatorAccountId(transaction.creatorAccountId());
        command_executor_->doValidation(false);
        auto execute_command = [this](auto &command) {
          auto result = boost::apply_visitor(*command_executor_, command.get());
          return result.match([](expected::Value<void> &v) { return true; },
                              [&](expected::Error<CommandError> &e) {
                                log_->error(e.error.toString());
                                return false;
                              });
        };
        return std::all_of(transaction.commands().begin(),
                           transaction.commands().end(),
                           execute_command);
      };

      *sql_ << "SAVEPOINT savepoint_";
      auto result = std::all_of(block.transactions().begin(),
                          block.transactions().end(),
                          execute_transaction);

      if (result) {
        block_store_.insert(std::make_pair(block.height(), clone(block)));
        block_index_->index(block);

        top_hash_ = block.hash();
        *sql_ << "RELEASE SAVEPOINT savepoint_";
      } else {
        *sql_ << "ROLLBACK TO SAVEPOINT savepoint_";
      }
      return result;
    }

    MutableStorageImpl::~MutableStorageImpl() {
      if (not committed) {
        *sql_ << "ROLLBACK";
      }
    }
  }  // namespace ametsuchi
}  // namespace iroha
