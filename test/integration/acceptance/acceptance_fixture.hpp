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
#include "interfaces/permissions.hpp"
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
      const shared_model::interface::types::AccountNameType &user,
      const shared_model::crypto::PublicKey &key);

  /**
   * Creates a set of transactions for user creation with specified permissions
   * @param user is username of new user
   * @param key is a public key of new user
   * @param role_id is new role of the user
   * @param perms is a collections of permissions of the user
   * @return pre-build transaction
   */
  TestUnsignedTransactionBuilder createUserWithPerms(
      const shared_model::interface::types::AccountNameType &user,
      const shared_model::crypto::PublicKey &key,
      const shared_model::interface::types::RoleIdType &role_id,
      const shared_model::interface::RolePermissionSet &perms);

  /**
   * Creates the transaction with the user creation commands
   * @param role_name is a name of the role
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  shared_model::proto::Transaction makeUserWithPerms(
      const shared_model::interface::types::RoleIdType &role_name,
      const shared_model::interface::RolePermissionSet &perms);

  /**
   * Creates the transaction with the user creation commands
   * @param perms are the permissions of the user
   * @return built tx and a hash of its payload
   */
  shared_model::proto::Transaction makeUserWithPerms(
      const shared_model::interface::RolePermissionSet &perms);

  /**
   * Add default user creator account id and current created time to builder
   * @tparam Builder type (transaction, query)
   * @param builder object to modify
   * @return builder containing creator account id and created time
   */
  template <typename Builder>
  auto base(Builder builder,
            const shared_model::interface::types::AccountIdType &account_id)
      -> decltype(
          builder
              .creatorAccountId(shared_model::interface::types::AccountIdType())
              .createdTime(uint64_t()));

  /**
   * Create valid base pre-built transaction with specified creator
   * @param account_id - account of transaction creator
   * @return pre-built tx
   */
  auto baseTx(const shared_model::interface::types::AccountIdType &account_id)
      -> decltype(base(TestUnsignedTransactionBuilder(),
                       shared_model::interface::types::AccountIdType()));

  /**
   * Create valid base pre-built transaction with kUserId as transaction creator
   * @return pre-built tx
   */
  auto baseTx()
      -> decltype(baseTx(shared_model::interface::types::AccountIdType()));

  /**
   * Create valid base pre-built query with specified query creator
   * @param account_id - account of query creator
   * @return pre-built query
   */
  auto baseQry(const shared_model::interface::types::AccountIdType &account_id)
      -> decltype(base(TestUnsignedQueryBuilder(), std::string()));

  /**
   * Create valid base pre-built query with kUserId as query creator
   * @return pre-built query
   */
  auto baseQry() -> decltype(baseQry(std::string()));

  /**
   * Completes pre-built object with specified keypair for signing
   * @tparam Builder - is a type of a pre-built object
   * @param builder - is a pre-built object
   * @param keypair - keypair used for signing
   * @return built object
   */
  template <typename Builder>
  auto complete(Builder builder, const shared_model::crypto::Keypair &keypair)
      -> decltype(builder.build()
                      .signAndAddSignature(
                          std::declval<shared_model::crypto::Keypair>())
                      .finish());

  /**
   * Completes pre-built object with kUserKeypair used for signing
   * @param builder is a pre-built object
   * @return built object
   */
  template <typename Builder>
  auto complete(Builder builder)
      -> decltype(builder.build()
      .signAndAddSignature(
          std::declval<shared_model::crypto::Keypair>())
      .finish());

  /**
   * @return unique time for this fixture
   */
  iroha::time::time_t getUniqueTime();

  const shared_model::interface::types::AccountNameType kUser;
  const shared_model::interface::types::RoleIdType kRole;
  const shared_model::interface::types::DomainIdType kDomain;
  const shared_model::interface::types::AssetIdType kAssetId;
  const shared_model::interface::types::AccountIdType kUserId;
  const shared_model::interface::types::AccountIdType kAdminId;
  const shared_model::crypto::Keypair kAdminKeypair;
  const shared_model::crypto::Keypair kUserKeypair;

  const std::function<void(const shared_model::proto::TransactionResponse &)>
      checkStatelessInvalid;

  const std::vector<shared_model::interface::types::AssetNameType>
      kIllegalAssetNames = {"",
                            " ",
                            "   ",
                            "A",
                            "assetV",
                            "asSet",
                            "asset%",
                            "^123",
                            "verylongassetname_thenameislonger",
                            "verylongassetname_thenameislongerthanitshouldbe",
                            "assset-01"};

  const std::vector<shared_model::interface::types::DomainIdType>
      kIllegalDomainNames = {
          "",
          " ",
          "   ",
          "9start.with.digit",
          "-startWithDash",
          "@.is.not.allowed",
          "no space is allowed",
          "endWith-",
          "label.endedWith-.is.not.allowed",
          "aLabelMustNotExceeds63charactersALabelMustNotExceeds63characters",
          "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
          "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
          "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
          "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPadP",
          "257.257.257.257",
          "domain#domain",
          "asd@asd",
          "ab..cd"};

 private:
  iroha::time::time_t initial_time;
  /// number of created transactions, used to provide unique time
  int nonce_counter;
};

#endif  // IROHA_ACCEPTANCE_FIXTURE_HPP
