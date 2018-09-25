/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "integration/acceptance/acceptance_fixture.hpp"

#include <utility>

#include "datetime/time.hpp"
#include "framework/specified_visitor.hpp"
#include "utils/query_error_response_visitor.hpp"

namespace {
  template <typename T>
  void checkStatus(integration_framework::IntegrationTestFramework::
                       TxResponseList::value_type &resp) {
    ASSERT_NO_THROW(
        boost::apply_visitor(framework::SpecifiedVisitor<T>(), resp->get()));
  }
}  // namespace

AcceptanceFixture::AcceptanceFixture()
    : kUser("user"),
      kRole("role"),
      kDomain(integration_framework::IntegrationTestFramework::kDefaultDomain),
      kAssetId(
          integration_framework::IntegrationTestFramework::kAssetName + "#"
          + integration_framework::IntegrationTestFramework::kDefaultDomain),
      kUserId(
          kUser + "@"
          + integration_framework::IntegrationTestFramework::kDefaultDomain),
      kAdminId(integration_framework::IntegrationTestFramework::kAdminId),
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
      checkForStatelessInvalid([](auto status_list) {
        ASSERT_EQ(status_list.size(), 2);
        checkStatus<shared_model::interface::EnoughSignaturesCollectedResponse>(
            status_list[0]);
        checkStatus<shared_model::interface::StatelessFailedTxResponse>(
            status_list[1]);
      }),
      checkForStatelessValid([](auto status_list) {
        ASSERT_GE(status_list.size(), 2);
        checkStatus<shared_model::interface::EnoughSignaturesCollectedResponse>(
            status_list[0]);
        checkStatus<shared_model::interface::StatelessValidTxResponse>(
            status_list[1]);
      }),
      checkForStatefulInvalid([](auto status_list) {
        ASSERT_EQ(status_list.size(), 3);
        checkStatus<shared_model::interface::EnoughSignaturesCollectedResponse>(
            status_list[0]);
        checkStatus<shared_model::interface::StatelessValidTxResponse>(
            status_list[1]);
        checkStatus<shared_model::interface::StatefulFailedTxResponse>(
            status_list[2]);
      }),
      checkForStatefulValid([](auto status_list) {
        ASSERT_GE(status_list.size(), 3);
        checkStatus<shared_model::interface::EnoughSignaturesCollectedResponse>(
            status_list[0]);
        checkStatus<shared_model::interface::StatelessValidTxResponse>(
            status_list[1]);
        checkStatus<shared_model::interface::StatefulValidTxResponse>(
            status_list[2]);
      }),
      checkForCommitted([](auto status_list) {
        ASSERT_EQ(status_list.size(), 4);
        checkStatus<shared_model::interface::EnoughSignaturesCollectedResponse>(
            status_list[0]);
        checkStatus<shared_model::interface::StatelessValidTxResponse>(
            status_list[1]);
        checkStatus<shared_model::interface::StatefulValidTxResponse>(
            status_list[2]);
        checkStatus<shared_model::interface::CommittedTxResponse>(
            status_list[3]);
      }),
      initial_time(iroha::time::now()),
      nonce_counter(1) {}

TestUnsignedTransactionBuilder AcceptanceFixture::createUser(
    const shared_model::interface::types::AccountNameType &user,
    const shared_model::crypto::PublicKey &key) {
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
    const shared_model::interface::types::AccountNameType &user,
    const shared_model::crypto::PublicKey &key,
    const shared_model::interface::types::RoleIdType &role_id,
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
    const shared_model::interface::types::RoleIdType &role_name,
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
auto AcceptanceFixture::base(
    Builder builder,
    const shared_model::interface::types::AccountIdType &account_id)
    -> decltype(
        builder
            .creatorAccountId(shared_model::interface::types::AccountIdType())
            .createdTime(uint64_t())) {
  return builder.creatorAccountId(account_id).createdTime(getUniqueTime());
}

template auto AcceptanceFixture::base<TestUnsignedTransactionBuilder>(
    TestUnsignedTransactionBuilder builder,
    const shared_model::interface::types::AccountIdType &account_id)
    -> decltype(
        builder
            .creatorAccountId(shared_model::interface::types::AccountIdType())
            .createdTime(uint64_t()));
template auto AcceptanceFixture::base<TestUnsignedQueryBuilder>(
    TestUnsignedQueryBuilder builder,
    const shared_model::interface::types::AccountIdType &account_id)
    -> decltype(
        builder
            .creatorAccountId(shared_model::interface::types::AccountIdType())
            .createdTime(uint64_t()));

auto AcceptanceFixture::baseTx(
    const shared_model::interface::types::AccountIdType &account_id)
    -> decltype(base(TestUnsignedTransactionBuilder(), std::string())) {
  return base(TestUnsignedTransactionBuilder(), account_id).quorum(1);
}

auto AcceptanceFixture::baseTx()
    -> decltype(baseTx(shared_model::interface::types::AccountIdType())) {
  return baseTx(kUserId);
}

auto AcceptanceFixture::baseQry(
    const shared_model::interface::types::AccountIdType &account_id)
    -> decltype(base(TestUnsignedQueryBuilder(), std::string())) {
  return base(TestUnsignedQueryBuilder(), account_id)
      .queryCounter(nonce_counter);
}

auto AcceptanceFixture::baseQry()
    -> decltype(baseQry(shared_model::interface::types::AccountIdType())) {
  return baseQry(kUserId);
}

template <typename Builder>
auto AcceptanceFixture::complete(Builder builder,
                                 const shared_model::crypto::Keypair &keypair)
    -> decltype(
        builder.build()
            .signAndAddSignature(std::declval<shared_model::crypto::Keypair>())
            .finish()) {
  return builder.build().signAndAddSignature(keypair).finish();
}

template auto AcceptanceFixture::complete<TestUnsignedTransactionBuilder>(
    TestUnsignedTransactionBuilder builder,
    const shared_model::crypto::Keypair &keypair)
    -> decltype(
        builder.build()
            .signAndAddSignature(std::declval<shared_model::crypto::Keypair>())
            .finish());
template auto AcceptanceFixture::complete<TestUnsignedQueryBuilder>(
    TestUnsignedQueryBuilder builder,
    const shared_model::crypto::Keypair &keypair)
    -> decltype(
        builder.build()
            .signAndAddSignature(std::declval<shared_model::crypto::Keypair>())
            .finish());

template <typename Builder>
auto AcceptanceFixture::complete(Builder builder)
    -> decltype(builder.build().finish()) {
  return complete(builder, kUserKeypair);
}

template <typename ErrorResponse>
std::function<void(const shared_model::interface::QueryResponse &)>
AcceptanceFixture::checkQueryErrorResponse() {
  return [](auto &response) {
    ASSERT_TRUE(boost::apply_visitor(
        shared_model::interface::QueryErrorResponseChecker<ErrorResponse>(),
        response.get()));
  };
}

template auto AcceptanceFixture::complete<TestUnsignedTransactionBuilder>(
    TestUnsignedTransactionBuilder builder)
    -> decltype(builder.build().finish());
template auto AcceptanceFixture::complete<TestUnsignedQueryBuilder>(
    TestUnsignedQueryBuilder builder) -> decltype(builder.build().finish());

template std::function<void(const shared_model::interface::QueryResponse &)>
AcceptanceFixture::checkQueryErrorResponse<
    shared_model::interface::StatelessFailedErrorResponse>();
template std::function<void(const shared_model::interface::QueryResponse &)>
AcceptanceFixture::checkQueryErrorResponse<
    shared_model::interface::StatefulFailedErrorResponse>();
template std::function<void(const shared_model::interface::QueryResponse &)>
AcceptanceFixture::checkQueryErrorResponse<
    shared_model::interface::NoAccountErrorResponse>();
template std::function<void(const shared_model::interface::QueryResponse &)>
AcceptanceFixture::checkQueryErrorResponse<
    shared_model::interface::NoAccountAssetsErrorResponse>();
template std::function<void(const shared_model::interface::QueryResponse &)>
AcceptanceFixture::checkQueryErrorResponse<
    shared_model::interface::NoAccountDetailErrorResponse>();
template std::function<void(const shared_model::interface::QueryResponse &)>
AcceptanceFixture::checkQueryErrorResponse<
    shared_model::interface::NoSignatoriesErrorResponse>();
template std::function<void(const shared_model::interface::QueryResponse &)>
AcceptanceFixture::checkQueryErrorResponse<
    shared_model::interface::NotSupportedErrorResponse>();
template std::function<void(const shared_model::interface::QueryResponse &)>
AcceptanceFixture::checkQueryErrorResponse<
    shared_model::interface::NoAssetErrorResponse>();
template std::function<void(const shared_model::interface::QueryResponse &)>
AcceptanceFixture::checkQueryErrorResponse<
    shared_model::interface::NoRolesErrorResponse>();

iroha::time::time_t AcceptanceFixture::getUniqueTime() {
  return initial_time + nonce_counter++;
}
