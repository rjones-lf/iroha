/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "integration/acceptance/acceptance_fixture.hpp"

#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "interfaces/utils/specified_visitor.hpp"

AcceptanceFixture::AcceptanceFixture()
    : kUser("user"),
      kRole("role"),
      kDomain(integration_framework::IntegrationTestFramework::kDefaultDomain),
      kAsset(integration_framework::IntegrationTestFramework::kAssetName + "#"
             + integration_framework::IntegrationTestFramework::kDefaultDomain),
      kUserId(
          kUser + "@"
          + integration_framework::IntegrationTestFramework::kDefaultDomain),
      kAdminKeypair(
          shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair()),
      kUserKeypair(
          shared_model::crypto::DefaultCryptoAlgorithmType::generateKeypair()),
      checkStatelessInvalid([](auto &status) {
        ASSERT_TRUE(boost::apply_visitor(
            shared_model::interface::SpecifiedVisitor<
                shared_model::interface::StatelessFailedTxResponse>(),
            status.get()));
      }) {}

TestUnsignedTransactionBuilder AcceptanceFixture::createUser(
    const std::string &user, const shared_model::crypto::PublicKey &key) {
  return TestUnsignedTransactionBuilder()
      .createAccount(
          user,
          integration_framework::IntegrationTestFramework::kDefaultDomain,
          key)
      .creatorAccountId(
          integration_framework::IntegrationTestFramework::kAdminId)
      .createdTime(iroha::time::now());
}

TestUnsignedTransactionBuilder AcceptanceFixture::createUserWithPerms(
    const std::string &user,
    const shared_model::crypto::PublicKey &key,
    const std::string &role_id,
    std::vector<std::string> perms) {
  const auto user_id = user + "@"
      + integration_framework::IntegrationTestFramework::kDefaultDomain;
  return createUser(user, key)
      .detachRole(user_id,
                  integration_framework::IntegrationTestFramework::kDefaultRole)
      .createRole(role_id, perms)
      .appendRole(user_id, role_id);
}

shared_model::proto::Transaction AcceptanceFixture::makeUserWithPerms(
    const std::string &role_name, const std::vector<std::string> &perms) {
  return createUserWithPerms(kUser, kUserKeypair.publicKey(), role_name, perms)
      .build()
      .signAndAddSignature(kAdminKeypair);
}

shared_model::proto::Transaction AcceptanceFixture::makeUserWithPerms(
    const std::vector<std::string> &perms) {
  return makeUserWithPerms(kRole, perms);
}

template <typename Builder>
auto AcceptanceFixture::base(Builder builder) -> decltype(
    builder.creatorAccountId(std::string()).createdTime(uint64_t())) {
  return builder.creatorAccountId(kUserId).createdTime(iroha::time::now());
}

template auto AcceptanceFixture::base<TestUnsignedTransactionBuilder>(
    TestUnsignedTransactionBuilder builder)
    -> decltype(
        builder.creatorAccountId(std::string()).createdTime(uint64_t()));
template auto AcceptanceFixture::base<TestUnsignedQueryBuilder>(
    TestUnsignedQueryBuilder builder)
    -> decltype(
        builder.creatorAccountId(std::string()).createdTime(uint64_t()));

auto AcceptanceFixture::baseTx()
    -> decltype(base(TestUnsignedTransactionBuilder())) {
  return base(TestUnsignedTransactionBuilder());
}

auto AcceptanceFixture::baseQry()
    -> decltype(base(TestUnsignedQueryBuilder())) {
  return base(TestUnsignedQueryBuilder());
}

template <typename Builder>
auto AcceptanceFixture::complete(Builder builder)
    -> decltype(builder.build().signAndAddSignature(
        std::declval<shared_model::crypto::Keypair>())) {
  return builder.build().signAndAddSignature(kUserKeypair);
}

template auto AcceptanceFixture::complete<TestUnsignedTransactionBuilder>(
    TestUnsignedTransactionBuilder builder)
    -> decltype(builder.build().signAndAddSignature(
        std::declval<shared_model::crypto::Keypair>()));
template auto AcceptanceFixture::complete<TestUnsignedQueryBuilder>(
    TestUnsignedQueryBuilder builder)
    -> decltype(builder.build().signAndAddSignature(
        std::declval<shared_model::crypto::Keypair>()));
