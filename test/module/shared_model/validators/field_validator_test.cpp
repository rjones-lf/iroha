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

#include <memory>
#include <unordered_set>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

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
    InitFieldFunction init_func;
    bool value_is_valid;
    std::string expected_message;
  };

 public:
  // To test one field, validation function is required,
  // which is always the same, and several test cases, which represent various
  // inputs
  using FieldTest = std::pair<ValidationFunction, std::vector<FieldTestCase>>;

 protected:
  // Because we use Protobuf reflection to generate fields by generating all
  // possible commands, some fields are checked several times
  // because they are present in several commands. To prevent that, this set
  // contains all already tested fields, and checked each time we try to test
  // a field
  std::unordered_set<std::string> checked_fields;

  /************************** TEST CASES ***************************/

  std::vector<FieldTestCase> account_id_test_cases{
      // valid
      {[&] { account_id = "account@domain"; }, true, ""},
      // cannot start with a digit
      {[&] { account_id = "1abs@domain"; },
       false,
       "Wrongly formed account_id, passed value: 1abs@domain"},
  };

  std::vector<FieldTestCase> asset_id_test_cases{
      // valid
      {[&] { asset_id = "asset#domain"; }, true, ""},
      // cannot start with a digit
      {[&] { asset_id = "1abs#domain"; },
       false,
       "Wrongly formed asset_id, passed value: 1abs#domain"},
  };

  std::vector<FieldTestCase> amount_test_cases;
  std::vector<FieldTestCase> address_test_cases;
  std::vector<FieldTestCase> peer_key_test_cases;
  std::vector<FieldTestCase> public_key_test_cases;
  std::vector<FieldTestCase> role_name_test_cases;
  std::vector<FieldTestCase> account_name_test_cases;
  std::vector<FieldTestCase> domain_id_test_cases;
  std::vector<FieldTestCase> main_pubkey_test_cases;
  std::vector<FieldTestCase> asset_name_test_cases;
  std::vector<FieldTestCase> precision_test_cases;
  std::vector<FieldTestCase> default_role_test_cases;
  std::vector<FieldTestCase> permission_test_cases;
  std::vector<FieldTestCase> permissions_test_cases;
  std::vector<FieldTestCase> key_test_cases;
  std::vector<FieldTestCase> value_test_cases;
  std::vector<FieldTestCase> quorum_test_cases;
  std::vector<FieldTestCase> src_account_id_test_cases;
  std::vector<FieldTestCase> dest_account_id_test_cases;
  std::vector<FieldTestCase> description_test_cases;
  std::vector<FieldTestCase> creator_account_id_test_cases;
  std::vector<FieldTestCase> tx_counter_test_cases;
  std::vector<FieldTestCase> created_time_test_cases;
  std::vector<FieldTestCase> pubkey_test_cases;
  std::vector<FieldTestCase> signature_test_cases;
  std::vector<FieldTestCase> role_id_test_cases;
  std::vector<FieldTestCase> detail_test_cases;
  std::vector<FieldTestCase> tx_hashes_test_cases;

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
          // field_validator.validate();
          return reason;
        },
        signature_test_cases}},
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
        tx_hashes_test_cases}}};
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
      [this](auto field, auto command) {
        // skip field, if already tested
        if (checked_fields.find(field->name()) != checked_fields.end()) {
          return;
        }
        checked_fields.insert(field->name());

        // Will throw key exception in case new field is added
        FieldTest field_test;
        try {
          field_test = field_validators.at(field->name());
        } catch (const std::out_of_range &e) {
          FAIL() << "Missing field setter: " << field->name();
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
            EXPECT_TRUE(!reason.second.empty());
            EXPECT_EQ(testcase.expected_message, reason.second.at(0));
          } else {
            EXPECT_TRUE(reason.second.empty());
          }
        }
      },
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

      [this](auto field, auto transaction_field) {
        // skip field, if already tested
        if (checked_fields.find(field->name()) != checked_fields.end()) {
          return;
        }
        checked_fields.insert(field->name());

        // skip field, if it is of complex type (like command)
        // these must be tested separately
        if (field->type()
            == google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE) {
          return;
        }

        // Will throw key exception in case new field is added
        FieldTest field_test;
        try {
          field_test = field_validators.at(field->name());
        } catch (const std::out_of_range &e) {
          FAIL() << "Missing field setter: " << field->name();
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
            EXPECT_TRUE(!reason.second.empty());
            EXPECT_EQ(testcase.expected_message, reason.second.at(0));
          } else {
            EXPECT_TRUE(reason.second.empty());
          }
        }
      },
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
      [this](auto field, auto query) {
        // skip field, if already tested
        if (checked_fields.find(field->name()) != checked_fields.end()) {
          return;
        }
        checked_fields.insert(field->name());

        // skip field, if it is of complex type (like command)
        // these must be tested separately
        if (field->type()
            == google::protobuf::FieldDescriptor::Type::TYPE_MESSAGE) {
          return;
        }

        // Will throw key exception in case new field is added
        FieldTest field_test;
        try {
          field_test = field_validators.at(field->name());
        } catch (const std::out_of_range &e) {
          FAIL() << "Missing field setter: " << field->name();
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
            EXPECT_TRUE(!reason.second.empty());
            EXPECT_EQ(testcase.expected_message, reason.second.at(0));
          } else {
            EXPECT_TRUE(reason.second.empty());
          }
        }
      },
      [&] {});
}
