/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "integration/acceptance/acceptance_fixture.hpp"

#include "datetime/time.hpp"
#include "framework/integration_framework/integration_test_framework.hpp"
#include "framework/specified_visitor.hpp"

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
        ASSERT_NO_THROW(boost::apply_visitor(
            framework::SpecifiedVisitor<
                shared_model::interface::StatelessFailedTxResponse>(),
            status.get()));
      }),
      initial_time(iroha::time::now()),
      nonce_counter(0) {}

TestUnsignedTransactionBuilder AcceptanceFixture::createUser(
    const std::string &user, const shared_model::crypto::PublicKey &key) {
  return TestUnsignedTransactionBuilder()
      .createAccount(
          user,
          integration_framework::IntegrationTestFramework::kDefaultDomain,
          key)
      .creatorAccountId(
          integration_framework::IntegrationTestFramework::kAdminId)
      .createdTime(getUniqueTime())
      .quorum(1);
}

TestUnsignedTransactionBuilder AcceptanceFixture::createUserWithPerms(
    const std::string &user,
    const shared_model::crypto::PublicKey &key,
    const std::string &role_id,
    const shared_model::interface::RolePermissionSet &perms) {
  const auto user_id = user + "@"
      + integration_framework::IntegrationTestFramework::kDefaultDomain;
  return createUser(user, key)
      .detachRole(user_id,
                  integration_framework::IntegrationTestFramework::kDefaultRole)
      .createRole(role_id, perms)
      .appendRole(user_id, role_id);
}

shared_model::proto::Transaction AcceptanceFixture::makeUserWithPerms(
    const std::string &role_name,
    const shared_model::interface::RolePermissionSet &perms) {
  return createUserWithPerms(kUser, kUserKeypair.publicKey(), role_name, perms)
      .build()
      .signAndAddSignature(kAdminKeypair)
      .finish();
}

shared_model::proto::Transaction AcceptanceFixture::makeUserWithPerms(
    const shared_model::interface::RolePermissionSet &perms) {
  return makeUserWithPerms(kRole, perms);
}

template <typename Builder>
auto AcceptanceFixture::base(Builder builder) -> decltype(
    builder.creatorAccountId(std::string()).createdTime(uint64_t())) {
  return builder.creatorAccountId(kUserId).createdTime(getUniqueTime());
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
  return base(TestUnsignedTransactionBuilder()).quorum(1);
}

auto AcceptanceFixture::baseQry()
    -> decltype(base(TestUnsignedQueryBuilder())) {
  return base(TestUnsignedQueryBuilder()).queryCounter(nonce_counter);
}

template <typename Builder>
auto AcceptanceFixture::complete(Builder builder) -> decltype(
    builder.build()
        .signAndAddSignature(std::declval<shared_model::crypto::Keypair>())
        .finish()) {
  return builder.build().signAndAddSignature(kUserKeypair).finish();
}

template auto AcceptanceFixture::complete<TestUnsignedTransactionBuilder>(
    TestUnsignedTransactionBuilder builder)
    -> decltype(
        builder.build()
            .signAndAddSignature(std::declval<shared_model::crypto::Keypair>())
            .finish());
template auto AcceptanceFixture::complete<TestUnsignedQueryBuilder>(
    TestUnsignedQueryBuilder builder)
    -> decltype(
        builder.build()
            .signAndAddSignature(std::declval<shared_model::crypto::Keypair>())
            .finish());

iroha::time::time_t AcceptanceFixture::getUniqueTime() {
  return initial_time + nonce_counter++;
}
