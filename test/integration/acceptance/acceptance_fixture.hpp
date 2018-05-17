/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_ACCEPTANCE_FIXTURE_HPP
#define IROHA_ACCEPTANCE_FIXTURE_HPP

#include <gtest/gtest.h>
#include <functional>
#include <string>
#include <vector>
#include "cryptography/keypair.hpp"
#include "module/shared_model/builders/protobuf/test_query_builder.hpp"
#include "module/shared_model/builders/protobuf/test_transaction_builder.hpp"

namespace shared_model {
  namespace proto {
    class TransactionResponse;
  }  // namespace proto
}  // namespace shared_model

/**
 * Common values (user, domain, asset)
 * and methods (create user, base transaction) for acceptance tests
 */
class AcceptanceFixture : public ::testing::Test {
 public:
  AcceptanceFixture();

  /**
   * Creates a set of transactions for user creation
   * @param user is username of new user
   * @param key is a public key of new user
   * @return pre-built transaction
   */
  TestUnsignedTransactionBuilder createUser(
      const std::string &user, const shared_model::crypto::PublicKey &key);

  /**
   * Creates a set of transactions for user creation with specified permissions
   * @param user is username of new user
   * @param key is a public key of new user
   * @param role_id is new role of the user
   * @param perms is a collections of permissions of the user
   * @return pre-build transaction
   */
  TestUnsignedTransactionBuilder createUserWithPerms(
      const std::string &user,
      const shared_model::crypto::PublicKey &key,
      const std::string &role_id,
      std::vector<std::string> perms);

  /**
   * Creates the transaction with the user creation commands
   * @param role_name is a name of the role
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  shared_model::proto::Transaction makeUserWithPerms(
      const std::string &role_name, const std::vector<std::string> &perms);

  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  shared_model::proto::Transaction makeUserWithPerms(
      const std::vector<std::string> &perms);

  /**
   * Add default user creator account id and current created time to builder
   * @tparam Builder type (transaction, query)
   * @param builder object to modify
   * @return builder containing creator account id and created time
   */
  template <typename Builder>
  auto base(Builder builder) -> decltype(
      builder.creatorAccountId(std::string()).createdTime(uint64_t()));

  /**
   * Create valid base pre-built transaction
   * @return pre-built tx
   */
  auto baseTx() -> decltype(base(TestUnsignedTransactionBuilder()));

  /**
   * Create valid base pre-built query
   * @return pre-built query
   */
  auto baseQry() -> decltype(base(TestUnsignedQueryBuilder()));

  /**
   * Completes pre-built object
   * @param builder is a pre-built object
   * @return built object
   */
  template <typename Builder>
  auto complete(Builder builder)
      -> decltype(builder.build().signAndAddSignature(
          std::declval<shared_model::crypto::Keypair>()));

  const std::string kUser;
  const std::string kRole;
  const std::string kDomain;
  const std::string kAsset;
  const std::string kUserId;
  const shared_model::crypto::Keypair kAdminKeypair;
  const shared_model::crypto::Keypair kUserKeypair;

  const std::function<void(const shared_model::proto::TransactionResponse &)>
      checkStatelessInvalid;
};

#endif  // IROHA_ACCEPTANCE_FIXTURE_HPP
