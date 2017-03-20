/*
Copyright Soramitsu Co., Ltd. 2016 All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <gtest/gtest.h>
#include <infra/protobuf/api.pb.h>
#include <transaction_builder/transaction_builder.hpp>
#include <util/exception.hpp>

using txbuilder::TransactionBuilder;
using type_signatures::Transfer;
using type_signatures::Domain;
using type_signatures::Account;
using type_signatures::Asset;
using type_signatures::SimpleAsset;
using type_signatures::Peer;

/***************************************************************************
  unset test (for simplicity, check Transfer only)
 ***************************************************************************/
TEST(transaction_builder, create_unset_valid_various_order) {
  Api::Domain domain;
  domain.set_ownerpublickey("pubkey1");
  domain.set_name("name");

  ASSERT_NO_THROW({
  auto txDomain = TransactionBuilder<Transfer<Domain>>()
    .setDomain(domain)
    .setReceiverPublicKey("receiver pubkey")
    .setSenderPublicKey("karin")
    .build();
  });
}

TEST(transaction_builder, create_unset_sender) {
  Api::Domain domain;
  domain.set_ownerpublickey("pubkey1");
  domain.set_name("name");

  ASSERT_THROW({
  auto txDomain = TransactionBuilder<Transfer<Domain>>()
    .setReceiverPublicKey("receiver pubkey")
    .setDomain(domain)
    .build();
  }, exception::txbuilder::UnsetBuildArgmentsException);
}

TEST(transaction_builder, create_unset_receiver) {
  Api::Domain domain;
  domain.set_ownerpublickey("pubkey1");
  domain.set_name("name");

  ASSERT_THROW({
  auto txDomain = TransactionBuilder<Transfer<Domain>>()
    .setSenderPublicKey("karin")
    .setDomain(domain)
    .build();
  }, exception::txbuilder::UnsetBuildArgmentsException);
}

TEST(transaction_builder, create_unset_object) {
  Api::Domain domain;
  domain.set_ownerpublickey("pubkey1");
  domain.set_name("name");

  ASSERT_THROW({
  auto txDomain = TransactionBuilder<Transfer<Domain>>()
    .setSenderPublicKey("karin")
    .setReceiverPublicKey("receiver pubkey")
    .build();
  }, exception::txbuilder::UnsetBuildArgmentsException);
}

TEST(transaction_builder, create_unset_noset) {
  Api::Domain domain;
  domain.set_ownerpublickey("pubkey1");
  domain.set_name("name");

  ASSERT_THROW({
  auto txDomain = TransactionBuilder<Transfer<Domain>>()
    .build();
  }, exception::txbuilder::UnsetBuildArgmentsException);
}

TEST(transaction_builder, create_unset_invalid_double_set) {
  Api::Domain domain;
  domain.set_ownerpublickey("pubkey1");
  domain.set_name("name");

  ASSERT_THROW({
  auto txDomain = TransactionBuilder<Transfer<Domain>>()
    .setReceiverPublicKey("receiver pubkey")
    .setDomain(domain)
    .setSenderPublicKey("karin")
    .setReceiverPublicKey("receiver pubkey")
    .build();
  }, exception::txbuilder::DuplicateSetArgmentException);
}