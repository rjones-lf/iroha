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

#include "validators/field_validator.hpp"
#include "builders/protobuf/transaction.hpp"
#include "module/shared_model/validators/validators_fixture.hpp"
#include "utils/lazy_initializer.hpp"

using namespace shared_model;

class FieldValidatorTest : public ValidatorsTest {
  validation::FieldValidator field_validator;

 protected:
  // Function which performs validation
  using ValidationFunction = std::function<validation::ReasonsGroupType()>;
  // Function which initializes field, allows to avoid templates when dealing
  // with various types of fields
  using InitFieldFunction = std::function<void()>;

  /**
   * FieldTestCase is a struct that represents one value of some field,
   * and expected validation result.
   */
  struct FieldTestCase {
    InitFieldFunction init_func;
    bool value_is_valid;
    std::string expected_message;
  };

  // To test one field, validation function is required,
  // which is always the same, and several test cases, which represent various
  // inputs
  using FieldTest = std::pair<ValidationFunction, std::vector<FieldTestCase>>;

  std::vector<FieldTestCase> account_id_test_cases{
      // valid
      {[&] { account_id = "account@domain"; }, true, ""},
      // cannot start with a digit
      {[&] { account_id = "1abs@domain"; },
       false,
       "Wrongly formed account_id, passed value: 1abs@domain"},
  };

  std::unordered_map<std::string, FieldTest> field_validators{
      {"account_id",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountId(reason, account_id);
          return reason;
        },
        account_id_test_cases}},
      {"asset_id",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAssetId(reason, asset_id);
          return reason;
        },
        {}}},
      {"amount",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAmount(reason, proto::Amount(amount));
          return reason;
        },
        {}}},
      {"address",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePeerAddress(reason, address_localhost);
          return reason;
        },
        {}}},
      {"peer_key",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePubkey(
              reason, interface::types::PubkeyType(public_key));
          return reason;
        },
        {}}},
      {"public_key",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePubkey(
              reason, interface::types::PubkeyType(public_key));
          return reason;
        },
        {}}},
      {"role_name",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateRoleId(reason, role_name);
          return reason;
        },
        {}}},
      {"account_name",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountName(reason, account_name);
          return reason;
        },
        {}}},
      {"domain_id",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateDomainId(reason, domain_id);
          return reason;
        },
        {}}},
      {"main_pubkey",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePubkey(
              reason, interface::types::PubkeyType(public_key));
          return reason;
        },
        {}}},
      {"asset_name",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAssetName(reason, asset_name);
          return reason;
        },
        {}}},
      {"precision",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePrecision(reason, precision);
          return reason;
        },
        {}}},
      {"default_role",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateRoleId(reason, role_name);
          return reason;
        },
        {}}},
      {"permission",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePermission(
              reason,
              iroha::protocol::GrantablePermission_Name(grantable_permission));
          return reason;
        },
        {}}},
      {"permissions",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePermissions(
              reason, {iroha::protocol::RolePermission_Name(role_permission)});
          return reason;
        },
        {}}},
      {"key",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePubkey(
              reason, interface::types::PubkeyType(public_key));
          return reason;
        },
        {}}},
      {"value",
       {[&] {
          // TODO: add validation to a value
          validation::ReasonsGroupType reason;
          //  field_validator.validateValue(reason, "");
          return reason;
        },
        {}}},
      {"quorum",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateQuorum(reason, quorum);
          return reason;
        },
        {}}},
      {"src_account_id",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountId(reason, account_id);
          return reason;
        },
        {}}},
      {"dest_account_id",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountId(reason, account_id);
          return reason;
        },
        {}}},
      {"description",
       {[&] {
          //  TODO: add validation to description
          validation::ReasonsGroupType reason;
          //  field_validator.validateDescription(reason, description);
          return reason;
        },
        {}}},
  };
};

/**
 * @given field values from test cases
 * @when field validator is invoked
 * @then field validator correctly rejects invalid values, and provides
 * meaningful message
 */
TEST_F(FieldValidatorTest, TestCasesValidation) {
  iroha::protocol::Transaction proto_tx;
  auto payload = proto_tx.mutable_payload();
  // TODO: generate fields only once
  iterateContainer(
      [] { return iroha::protocol::Command::descriptor(); },
      [&](auto field) {
        // Add new command to transaction
        auto command = payload->add_commands();
        //  // Set concrete type for new command
        return command->GetReflection()->MutableMessage(command, field);
      },
      [this](auto field, auto command) {
        // Will throw key exception in case new field is added
        auto field_test = field_validators.at(field->name());
        auto validate = field_test.first;
        for (auto &testcase : field_test.second) {
          // Initialize field
          testcase.init_func();
          // Perform validation
          auto reason = validate();
          // if value supposed to be invalid, check that there is a reason
          // and that error message is as expected.
          // If value supposed to be valid, check for empty reason.
          if (!testcase.value_is_valid) {
            EXPECT_TRUE(!reason.second.empty());
            EXPECT_EQ(testcase.expected_message, reason.second.at(0));
          } else {
            EXPECT_TRUE(reason.second.empty());
          }
        }
      },
      [] {});
}
