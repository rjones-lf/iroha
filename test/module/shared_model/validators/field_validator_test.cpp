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

#include <limits>
#include <memory>
#include <unordered_set>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <boost/format.hpp>

#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "module/shared_model/validators/validators_fixture.hpp"
#include "utils/lazy_initializer.hpp"
#include "validators/field_validator.hpp"

using namespace shared_model;

class FieldValidatorTest : public ValidatorsTest {
  validation::FieldValidator field_validator;

 protected:
  // Function which performs validation
  using ValidationFunction = std::function<validation::ReasonsGroupType()>;
  // Function which initializes field, allows to have one type when dealing
  // with various types of fields
  using InitFieldFunction = std::function<void()>;

  /**
   * FieldTestCase is a struct that represents one value of some field,
   * and expected validation result.
   */
  struct FieldTestCase {
    std::string name;
    InitFieldFunction init_func;
    bool value_is_valid;
    std::string expected_message;
  };

  // Returns string containing field name and test case name for debug output
  std::string testFailMessage(const std::string &field_name,
                              const std::string &testcase_name) const {
    return (boost::format("Field: %s\nTest Case: %s\n") % field_name
            % testcase_name)
        .str();
  }

 public:
  // To test one field, validation function is required,
  // which is always the same, and several test cases, which represent
  // various inputs
  using FieldTest = std::pair<ValidationFunction, std::vector<FieldTestCase>>;

  // Run all test cases for given field
  void runTestCases(const google::protobuf::FieldDescriptor *field) {
    auto field_name = field->name();
    // skip field, if already tested
    if (checked_fields.find(field_name) != checked_fields.end()) {
      return;
    }
    checked_fields.insert(field_name);

    // Will throw key exception in case new field is added
    FieldTest field_test;
    try {
      field_test = field_validators.at(field_name);
    } catch (const std::out_of_range &e) {
      FAIL() << "Missing field setter: " << field_name;
    }
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
        ASSERT_TRUE(!reason.second.empty())
            << testFailMessage(field_name, testcase.name);
        EXPECT_EQ(testcase.expected_message, reason.second.at(0))
            << testFailMessage(field_name, testcase.name);
      } else {
        EXPECT_TRUE(reason.second.empty())
            << testFailMessage(field_name, testcase.name)
            << "Message: " << reason.second.at(0) << "\n";
      }
    }
  }

 protected:
  // Because we use Protobuf reflection to generate fields by generating all
  // possible commands, some fields are checked several times
  // because they are present in several commands. To prevent that, this set
  // contains all already tested fields, and checked each time we try to test
  // a field
  std::unordered_set<std::string> checked_fields;

  /************************** TEST CASES ***************************/

  std::vector<FieldTestCase> account_id_test_cases{
      {
          "valid",
          [&] { account_id = "account@domain"; },
          true,
          "",
      },
      {
          "start_with_digit",
          [&] { account_id = "1abs@domain"; },
          false,
          "Wrongly formed account_id, passed value: '1abs@domain'",
      },
      {
          "domain_start_with_digit",
          [&] { account_id = "account@3domain"; },
          false,
          "Wrongly formed account_id, passed value: 'account@3domain'",
      },
      {
          "empty_string",
          [&] { account_id = ""; },
          false,
          "Wrongly formed account_id, passed value: ''",
      },
      {
          "illegal_char",
          [&] { account_id = "abs^@domain"; },
          false,
          "Wrongly formed account_id, passed value: 'abs^@domain'",
      },
      {
          "missing_@_char",
          [&] { account_id = "absdomain"; },
          false,
          "Wrongly formed account_id, passed value: 'absdomain'",
      },
      {
          "missing_name",
          [&] { account_id = "@domain"; },
          false,
          "Wrongly formed account_id, passed value: '@domain'",
      },
  };

  std::vector<FieldTestCase> &src_account_id_test_cases = account_id_test_cases;
  std::vector<FieldTestCase> &dest_account_id_test_cases =
      account_id_test_cases;
  std::vector<FieldTestCase> &creator_account_id_test_cases =
      account_id_test_cases;

  std::vector<FieldTestCase> asset_id_test_cases{
      {
          "valid",
          [&] { asset_id = "asset#domain"; },
          true,
          "",
      },
      {
          "start_with_digit",
          [&] { asset_id = "1abs#domain"; },
          false,
          "Wrongly formed asset_id, passed value: '1abs#domain'",
      },
      {
          "domain_start_with_digit",
          [&] { asset_id = "abs#3domain"; },
          false,
          "Wrongly formed asset_id, passed value: 'abs#3domain'",
      },
      {
          "empty_string",
          [&] { asset_id = ""; },
          false,
          "Wrongly formed asset_id, passed value: ''",
      },
      {
          "illegal_char",
          [&] { asset_id = "ab++s#do()main"; },
          false,
          "Wrongly formed asset_id, passed value: 'ab++s#do()main'",
      },
      {
          "missing_#",
          [&] { asset_id = "absdomain"; },
          false,
          "Wrongly formed asset_id, passed value: 'absdomain'",
      },
      {
          "missing_asset",
          [&] { asset_id = "#domain"; },
          false,
          "Wrongly formed asset_id, passed value: '#domain'",
      },
  };

  std::vector<FieldTestCase> amount_test_cases{
      {
          "valid_amount",
          [&] { amount.mutable_value()->set_fourth(100); },
          true,
          "",
      },
      {
          "zero_amount",
          [&] { amount.mutable_value()->set_fourth(0); },
          false,
          "Amount must be greater than 0",
      },
  };

  // Address validation test is handled in libs/validator,
  // so test cases are not exhaustive
  std::vector<FieldTestCase> address_test_cases{
      {
          "valid_ip_address",
          [&] { address_localhost = "182.13.35.1:3040"; },
          true,
          "",
      },
      {
          "invalid_ip_address",
          [&] { address_localhost = "182.13.35.1:3040^^"; },
          false,
          "Wrongly formed peer address, passed value: '182.13.35.1:3040^^'",
      },
      {
          "empty_string",
          [&] { address_localhost = ""; },
          false,
          "Wrongly formed peer address, passed value: ''",
      },
  };

  std::vector<FieldTestCase> public_key_test_cases{
      {
          "valid_key",
          [&] { public_key = std::string(32, '0'); },
          true,
          "",
      },
      {
          "invalid_key_length",
          [&] { public_key = std::string(64, '0'); },
          false,
          "Public key has wrong size, passed size: 64",
      },
      {
          "empty_string",
          [&] { public_key = ""; },
          false,
          "Public key has wrong size, passed size: 0",
      },
  };
  // All public keys are currently the same, and follow the same rules
  std::vector<FieldTestCase> &peer_key_test_cases = public_key_test_cases;
  std::vector<FieldTestCase> &pubkey_test_cases = public_key_test_cases;
  std::vector<FieldTestCase> &main_pubkey_test_cases = public_key_test_cases;

  std::vector<FieldTestCase> role_name_test_cases{
      {
          "valid_name",
          [&] { role_name = "admin"; },
          true,
          "",
      },
      {
          "empty_string",
          [&] { role_name = ""; },
          false,
          "Wrongly formed role_id, passed value: ''",
      },
      {
          "illegal_characters",
          [&] { role_name = "+math+"; },
          false,
          "Wrongly formed role_id, passed value: '+math+'",
      },
      {
          "name_too_long",
          [&] { role_name = "somelongname"; },
          false,
          "Wrongly formed role_id, passed value: 'somelongname'",
      },
  };

  std::vector<FieldTestCase> &default_role_test_cases = role_name_test_cases;
  std::vector<FieldTestCase> &role_id_test_cases = role_name_test_cases;

  std::vector<FieldTestCase> account_name_test_cases{
      {
          "valid_name",
          [&] { account_name = "admin"; },
          true,
          "",
      },
      {
          "empty_string",
          [&] { account_name = ""; },
          false,
          "Wrongly formed account_name, passed value: ''",
      },
      {
          "illegal_char",
          [&] { account_name = "+math+"; },
          false,
          "Wrongly formed account_name, passed value: '+math+'",
      },
      {
          "name_too_long",
          [&] { account_name = "somelongname"; },
          false,
          "Wrongly formed account_name, passed value: 'somelongname'",
      },
  };

  std::vector<FieldTestCase> domain_id_test_cases{
      {
          "valid_id",
          [&] { domain_id = "admin"; },
          true,
          "",
      },
      {
          "empty_string",
          [&] { domain_id = ""; },
          false,
          "Wrongly formed domain_id, passed value: ''",
      },
      {
          "illegal_char",
          [&] { domain_id = "+math+"; },
          false,
          "Wrongly formed domain_id, passed value: '+math+'",
      },
      {
          "id_too_long",
          [&] { domain_id = "somelongname"; },
          false,
          "Wrongly formed domain_id, passed value: 'somelongname'",
      },
  };

  std::vector<FieldTestCase> asset_name_test_cases{
      {
          "valid_name",
          [&] { asset_name = "admin"; },
          true,
          "",
      },
      {
          "empty_string",
          [&] { asset_name = ""; },
          false,
          "Wrongly formed asset_name, passed value: ''",
      },
      {
          "illegal_char",
          [&] { asset_name = "+math+"; },
          false,
          "Wrongly formed asset_name, passed value: '+math+'",
      },
      {
          "name_too_long",
          [&] { asset_name = "somelongname"; },
          false,
          "Wrongly formed asset_name, passed value: 'somelongname'",
      },
  };

  std::vector<FieldTestCase> permissions_test_cases{
      {
          "valid_role",
          [&] {
            role_permission = iroha::protocol::RolePermission::can_append_role;
          },
          true,
          "",
      },
  };

  std::vector<FieldTestCase> tx_counter_test_cases{
      {
          "valid_counter",
          [&] { counter = 5; },
          true,
          "",
      },
      {
          "zero_counter",
          [&] { counter = 0; },
          false,
          "Counter should be > 0",
      },
  };
  std::vector<FieldTestCase> created_time_test_cases{
      {
          "valid_time",
          [&] { created_time = iroha::time::now(); },
          true,
          "",
      },
  };

  std::vector<FieldTestCase> detail_test_cases{
      {
          "valid_detail_key",
          [&] { detail_key = "happy"; },
          true,
          "",
      },
      {
          "empty_string",
          [&] { detail_key = ""; },
          false,
          "Wrongly formed key, passed value: ''",
      },
      {
          "illegal_char",
          [&] { detail_key = "hi*there"; },
          false,
          "Wrongly formed key, passed value: 'hi*there'",
      },
  };
  std::vector<FieldTestCase> &key_test_cases = detail_test_cases;

  // no constraints yet
  std::vector<FieldTestCase> precision_test_cases;
  std::vector<FieldTestCase> permission_test_cases;
  std::vector<FieldTestCase> value_test_cases;
  std::vector<FieldTestCase> quorum_test_cases;
  std::vector<FieldTestCase> description_test_cases;
  std::vector<FieldTestCase> signature_test_cases;
  std::vector<FieldTestCase> tx_hashes_test_cases;

  /**************************************************************************/

  // register validation function and test cases
  std::unordered_map<std::string, FieldTest> field_validators{
      // Command fields
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
        asset_id_test_cases}},
      {"amount",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAmount(reason, proto::Amount(amount));
          return reason;
        },
        amount_test_cases}},
      {"address",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePeerAddress(reason, address_localhost);
          return reason;
        },
        address_test_cases}},
      {"peer_key",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePubkey(
              reason, interface::types::PubkeyType(public_key));
          return reason;
        },
        peer_key_test_cases}},
      {"public_key",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePubkey(
              reason, interface::types::PubkeyType(public_key));
          return reason;
        },
        public_key_test_cases}},
      {"role_name",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateRoleId(reason, role_name);
          return reason;
        },
        role_name_test_cases}},
      {"account_name",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountName(reason, account_name);
          return reason;
        },
        account_name_test_cases}},
      {"domain_id",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateDomainId(reason, domain_id);
          return reason;
        },
        domain_id_test_cases}},
      {"main_pubkey",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePubkey(
              reason, interface::types::PubkeyType(public_key));
          return reason;
        },
        main_pubkey_test_cases}},
      {"asset_name",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAssetName(reason, asset_name);
          return reason;
        },
        asset_name_test_cases}},
      {"precision",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePrecision(reason, precision);
          return reason;
        },
        precision_test_cases}},
      {"default_role",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateRoleId(reason, role_name);
          return reason;
        },
        default_role_test_cases}},
      {"permission",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePermission(
              reason,
              iroha::protocol::GrantablePermission_Name(grantable_permission));
          return reason;
        },
        permission_test_cases}},
      {"permissions",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePermissions(
              reason, {iroha::protocol::RolePermission_Name(role_permission)});
          return reason;
        },
        permissions_test_cases}},
      {"key",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountDetailKey(reason, detail_key);
          return reason;
        },
        key_test_cases}},
      {"value",
       {[&] {
          // TODO: add validation to a value
          validation::ReasonsGroupType reason;
          //  field_validator.validateValue(reason, "");
          return reason;
        },
        value_test_cases}},
      {"quorum",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateQuorum(reason, quorum);
          return reason;
        },
        quorum_test_cases}},
      {"src_account_id",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountId(reason, account_id);
          return reason;
        },
        src_account_id_test_cases}},
      {"dest_account_id",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountId(reason, account_id);
          return reason;
        },
        dest_account_id_test_cases}},
      {"description",
       {[&] {
          //  TODO: add validation to description
          validation::ReasonsGroupType reason;
          //  field_validator.validateDescription(reason, description);
          return reason;
        },
        description_test_cases}},

      // Transaction fields
      {"creator_account_id",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountId(reason, account_id);
          return reason;
        },
        creator_account_id_test_cases}},
      {"tx_counter",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateCounter(reason, counter);
          return reason;
        },
        tx_counter_test_cases}},
      {"created_time",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateCreatedTime(reason, created_time);
          return reason;
        },
        created_time_test_cases}},
      {"pubkey",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validatePubkey(
              reason, interface::types::PubkeyType(public_key));
          return reason;
        },
        pubkey_test_cases}},
      {"signature",
       {[&] {
          validation::ReasonsGroupType reason;
          return reason;
        },
        signature_test_cases}},
      {"commands",
       {[&] {
          validation::ReasonsGroupType reason;
          return reason;
        },
        {}}},

      // Query fields
      {"role_id",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateRoleId(reason, role_name);
          return reason;
        },
        role_id_test_cases}},
      {"detail",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountDetailKey(reason, detail_key);
          return reason;
        },
        detail_test_cases}},
      {"tx_hashes",
       {[&] {
          validation::ReasonsGroupType reason;
          field_validator.validateAccountDetailKey(reason, detail_key);
          return reason;
        },
        tx_hashes_test_cases}},
  };
};

/**
 * @given field values from test cases
 * @when field validator is invoked on command's fields
 * @then field validator correctly rejects invalid values, and provides
 * meaningful message
 */
TEST_F(FieldValidatorTest, CommandFieldsValidation) {
  iroha::protocol::Transaction proto_tx;
  auto payload = proto_tx.mutable_payload();

  // iterate over all commands in transaction
  iterateContainer(
      [] { return iroha::protocol::Command::descriptor(); },
      [&](auto field) {
        // Add new command to transaction
        auto command = payload->add_commands();
        //  // Set concrete type for new command
        return command->GetReflection()->MutableMessage(command, field);
      },
      [this](auto field, auto command) { this->runTestCases(field); },
      [] {});
}

/**
 * @given field values from test cases
 * @when field validator is invoked on transaction's fields
 * @then field validator correctly rejects invalid values, and provides
 * meaningful message
 */
TEST_F(FieldValidatorTest, TransactionFieldsValidation) {
  // iterate over all fields in transaction
  iterateContainer(
      [] { return iroha::protocol::Transaction::descriptor(); },
      [&](auto field) {
        // generate message from field of transaction
        google::protobuf::DynamicMessageFactory message_factory;
        auto field_desc = field->message_type();
        // will be null if field is not of message type
        EXPECT_NE(nullptr, field_desc);
        return message_factory.GetPrototype(field_desc)->New();
      },
      [this](auto field, auto transaction_field) { this->runTestCases(field); },
      [] {});
}

/**
 * @given field values from test cases
 * @when field validator is invoked on query's fields
 * @then field validator correctly rejects invalid values, and provides
 * meaningful message
 */
TEST_F(FieldValidatorTest, QueryFieldsValidation) {
  iroha::protocol::Query qry;
  auto payload = qry.mutable_payload();
  // iterate over all field in query
  iterateContainer(
      [] {
        return iroha::protocol::Query::Payload::descriptor()->FindOneofByName(
            "query");
      },
      [&](auto field) {
        // Set concrete type for new query
        return payload->GetReflection()->MutableMessage(payload, field);
      },
      [this](auto field, auto query) { this->runTestCases(field); },
      [&] {});
}
