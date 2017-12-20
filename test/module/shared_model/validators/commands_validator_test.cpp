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

#include <gtest/gtest.h>
#include <boost/range/irange.hpp>
#include "builders/protobuf/transaction.hpp"
#include "utils/polymorphic_wrapper.hpp"


class CommandsValidatorTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    valid_created_time = iroha::time::now();

    auto public_key_size = 32;
    valid_public_key = std::string(public_key_size, '0');
  }
  std::string valid_account_id = "account@domain";
  std::string valid_asset_name = "asset";
  std::string valid_asset_id = "asset#domain";
  std::string valid_address_localhost = "localhost:65535";
  std::string valid_address_ipv4 = "192.168.255.1:8080";
  std::string valid_address_hostname = "google.ru:8080";
  std::string valid_role_name = "user";
  std::string valid_account_name = "admin";
  std::string valid_domain_id = "ru";
  iroha::protocol::GrantablePermission valid_grantable_permission =
      iroha::protocol::GrantablePermission::can_add_my_signatory;
  uint8_t valid_quorum = 2;
  std::string valid_amount = "12.34";
  std::string valid_description = "this is meaningless description";
  iroha::ts64_t valid_created_time;
  std::string valid_public_key;
  uint8_t valid_precision = 42;

  iroha::protocol::Transaction generateEmptyTransaction() {
    shared_model::interface::types::CounterType tx_counter = 1;
    std::string creator_account_id = "admin@test";

    iroha::protocol::Transaction proto_tx;
    auto &payload = *proto_tx.mutable_payload();
    payload.set_tx_counter(tx_counter);
    payload.set_creator_account_id(creator_account_id);
    payload.set_created_time(valid_created_time);
    return proto_tx;
  }
};

using namespace iroha::protocol;
using namespace shared_model;

/**
 * @given transaction without any commands
 * @when commands validator is invoked
 * @then answer has error about empty transaction
 */
TEST_F(CommandsValidatorTest, EmptyTransactionTest) {
  auto tx = generateEmptyTransaction();
  tx.mutable_payload()->set_created_time(iroha::time::now());
  shared_model::validation::CommandsValidator commands_validator;
  auto answer = commands_validator.validate(
      detail::makePolymorphic<proto::Transaction>(tx));
  ASSERT_EQ(answer.getReasonsMap().size(), 1);
}

/**
 * @given transaction made of commands with valid fields
 * @when commands validation is invoked
 * @then answer has no errors
 */
TEST_F(CommandsValidatorTest, StatelessValidTest) {
  // Valid values for fields in commands
  std::string valid_account_id = "account@domain";
  std::string valid_asset_id = "asset#domain";
  std::string valid_address = "localhost:8080";
  iroha::protocol::Amount valid_amount;
  valid_amount.set_precision(2);
  valid_amount.mutable_value()->set_fourth(1000);
  auto public_key_size = 32;
  std::string valid_public_key(public_key_size, '0');
  std::string valid_role_name = "user";
  std::string valid_account_name = "admin";
  std::string valid_domain_id = "ru";
  std::string valid_asset_name = "asset";
  uint8_t valid_precision = 42;
  iroha::protocol::RolePermission valid_role_permission =
      iroha::protocol::RolePermission::can_append_role;
  iroha::protocol::GrantablePermission valid_grantable_permission =
      iroha::protocol::GrantablePermission::can_add_my_signatory;
  std::string valid_detail_key = "key";
  uint8_t valid_quorum = 2;
  auto valid_created_time = iroha::time::now();

  // Generate protobuf reflection setter for given type and value
  auto setField = [&](auto setter) {
    return [setter](const auto &value) {
      return [setter, &value](auto refl, auto msg, auto field) {
        (refl->*setter)(msg, field, value);
      };
    };
  };

  auto setString = setField(&google::protobuf::Reflection::SetString);
  auto setUInt32 = setField(&google::protobuf::Reflection::SetUInt32);
  auto addEnum = setField(&google::protobuf::Reflection::AddEnumValue);
  auto setEnum = setField(&google::protobuf::Reflection::SetEnumValue);

  // List all used fields in commands
  std::unordered_map<
      std::string,
      std::function<void(const google::protobuf::Reflection *,
                         google::protobuf::Message *,
                         const google::protobuf::FieldDescriptor *)>>
      field_setters;
  for (const auto &id : {"account_id", "src_account_id", "dest_account_id"}) {
    field_setters[id] = setString(valid_account_id);
  }
  for (const auto &id : {"peer_key", "public_key", "main_pubkey"}) {
    field_setters[id] = setString(valid_public_key);
  }
  for (const auto &id : {"role_name", "default_role"}) {
    field_setters[id] = setString(valid_role_name);
  }
  field_setters["asset_id"] = setString(valid_asset_id);
  field_setters["address"] = setString(valid_address);
  field_setters["account_name"] = setString(valid_account_name);
  field_setters["domain_id"] = setString(valid_domain_id);
  field_setters["asset_name"] = setString(valid_asset_name);
  field_setters["precision"] = setUInt32(valid_precision);
  field_setters["permissions"] = addEnum(valid_role_permission);
  field_setters["permission"] = setEnum(valid_grantable_permission);
  field_setters["key"] = setString(valid_detail_key);
  field_setters["value"] = setString("");
  field_setters["quorum"] = setUInt32(valid_quorum);
  field_setters["description"] = setString("");
  field_setters["amount"] = [&](auto refl, auto msg, auto field) {
    refl->MutableMessage(msg, field)->CopyFrom(valid_amount);
  };

  iroha::protocol::Transaction tx = generateEmptyTransaction();
  tx.mutable_payload()->set_creator_account_id(valid_account_id);
  tx.mutable_payload()->set_created_time(valid_created_time);
  auto payload = tx.mutable_payload();

  // Iterate through all command types, filling command fields with valid values
  auto desc = iroha::protocol::Command::descriptor();
  boost::for_each(boost::irange(0, desc->field_count()), [&](auto i) {
    // Get field descriptor for concrete command (add asset quantity, etc.)
    auto field = desc->field(i);
    // Add new command to transaction
    auto command_variant = payload->add_commands();
    // Set concrete type for new command
    auto command = command_variant->GetReflection()->MutableMessage(
        command_variant, field);

    // Iterate through all fields of concrete command
    auto command_desc = command->GetDescriptor();
    boost::for_each(boost::irange(0, command_desc->field_count()), [&](auto i) {
      // Get field descriptor for concrete command field (account_id, etc.)
      auto field = command_desc->field(i);

      // Will throw key exception in case new field is added
      field_setters.at(field->name())(command->GetReflection(), command, field);
    });
  });

  shared_model::validation::CommandsValidator commands_validator;
  auto answer = commands_validator.validate(
      detail::makePolymorphic<proto::Transaction>(tx));

  ASSERT_FALSE(answer.hasErrors());
}

/**
 * @given transaction made of commands with invalid fields
 * @when commands validation is invoked
 * @then answer has errors and number of errors in answer is the same as the
 * number of commands in tx
 */
TEST_F(CommandsValidatorTest, StatelessInvalidTest) {
  iroha::protocol::Transaction tx = generateEmptyTransaction();
  auto payload = tx.mutable_payload();

  iroha::ts64_t invalid_time = 10000000000ull;;
  payload->set_created_time(invalid_time);

  // create commands from default constructors, which will have empty, therefore
  // invalid values
  auto desc = iroha::protocol::Command::descriptor();
  boost::for_each(boost::irange(0, desc->field_count()), [&](auto i) {
    // Get field descriptor for concrete command (add asset quantity, etc.)
    auto field = desc->field(i);
    // Add new command to transaction
    auto command = payload->add_commands();
    // Set concrete type for new command
    command->GetReflection()->MutableMessage(command, field);
    // Note that no fields are set
  });

  shared_model::validation::CommandsValidator commands_validator;
  auto answer = commands_validator.validate(
      detail::makePolymorphic<proto::Transaction>(tx));

  // in total there should be number_of_commands + 1 reasons of bad answer:
  // number_of_commands for each command + 1 for transaction metadata
  ASSERT_EQ(answer.getReasonsMap().size(), desc->field_count() + 1);
}

TEST_F(CommandsValidatorTest, AddressTest) {
  auto valid_addresses = {"localhost:8080", "192.168.0.255:65535", "soramitsu.co.jp:8080"};
  auto invalid_addresses = {"localhost::8080", "192.168.0.256:8080", "192.168.0.255:65536", "soramitsu.co.:8080"};

  AddPeer add_peer;
  add_peer.set_peer_key(valid_public_key);

  shared_model::validation::CommandsValidator commands_validator;
  for (auto address: valid_addresses){
    auto tx = generateEmptyTransaction();
    AddPeer valid_add_peer(add_peer);
    valid_add_peer.set_address(address);
    tx.mutable_payload()->add_commands()->mutable_add_peer()->CopyFrom(valid_add_peer);
    auto answer = commands_validator.validate(
        detail::make_polymorphic<proto::Transaction>(tx));
    if (answer.hasErrors()) {
      ASSERT_FALSE(answer.hasErrors());
    }
  }

  for (auto address: invalid_addresses){
    auto tx = generateEmptyTransaction();
    AddPeer invalid_add_peer(add_peer);
    invalid_add_peer.set_address(address);
    tx.mutable_payload()->add_commands()->mutable_add_peer()->CopyFrom(invalid_add_peer);
    auto answer = commands_validator.validate(
        detail::make_polymorphic<proto::Transaction>(tx));
    ASSERT_TRUE(answer.hasErrors());
  }

}
