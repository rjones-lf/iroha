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

#include <gmock/gmock-matchers.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>
#include <boost/format.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/join.hpp>

#include "backend/protobuf/common_objects/peer.hpp"
#include "builders/protobuf/queries.hpp"
#include "builders/protobuf/transaction.hpp"
#include "module/shared_model/validators/validators_fixture.hpp"
#include "utils/lazy_initializer.hpp"
#include "validators/field_validator.hpp"
#include "validators/permissions.hpp"

using namespace shared_model;

class FieldValidatorTest : public ValidatorsTest {
  validation::FieldValidator field_validator;

 protected:
  // Function which performs validation
  using ValidationFunction = std::function<validation::ReasonsGroupType()>;
  // Function which initializes field, allows to have one type when dealing
  // with various types of fields
  using InitFieldFunction = std::function<void()>;

  // Gaps for checking transactions from future
  static auto constexpr nearest_future =
      std::chrono::minutes(3) / std::chrono::milliseconds(1);
  static auto constexpr far_future =
      std::chrono::minutes(30) / std::chrono::milliseconds(1);

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
  FieldValidatorTest() {
    for (const auto &field : {"public_key", "main_pubkey", "pubkey"}) {
      field_validators.insert(makeTransformValidator(
          field,
          &FieldValidator::validatePubkey,
          &FieldValidatorTest::public_key,
          [](auto &&x) { return interface::types::PubkeyType(x); },
          public_key_test_cases));
    }
    for (const auto &field : {"role_name", "default_role", "role_id"}) {
      field_validators.insert(makeValidator(field,
                                            &FieldValidator::validateRoleId,
                                            &FieldValidatorTest::role_name,
                                            role_name_test_cases));
    }
    for (const auto &field : {"account_id",
                              "src_account_id",
                              "dest_account_id",
                              "creator_account_id"}) {
      field_validators.insert(makeValidator(field,
                                            &FieldValidator::validateAccountId,
                                            &FieldValidatorTest::account_id,
                                            account_id_test_cases));
    }

    field_validators.insert(
        makeValidator("key",
                      &FieldValidator::validateAccountDetailKey,
                      &FieldValidatorTest::detail_key,
                      detail_key_test_cases));

    field_validators.insert(
        makeValidator("value",
                      &FieldValidator::validateAccountDetailValue,
                      &FieldValidatorTest::detail_value,
                      detail_value_test_cases));

    field_validators.insert(makeValidator("domain_id",
                                          &FieldValidator::validateDomainId,
                                          &FieldValidatorTest::domain_id,
                                          domain_id_test_cases));
    for (const auto &field : {"query_counter"}) {
      field_validators.insert(makeValidator(field,
                                            &FieldValidator::validateCounter,
                                            &FieldValidatorTest::counter,
                                            counter_test_cases));
    }

    field_validators.insert(makeValidator("quorum",
                                          &FieldValidator::validateQuorum,
                                          &FieldValidatorTest::quorum,
                                          quorum_test_cases));

    field_validators.insert(makeValidator("permission",
                                          &FieldValidator::validatePermission,
                                          &FieldValidatorTest::permission,
                                          permission_test_cases));

    // TODO: add validation to all fields
    for (const auto &field : {"value",
                              "signature",
                              "commands",
                              "quorum",
                              "tx_hashes",
                              "precision"}) {
      field_validators.insert(makeNullValidator(field));
    }
  }

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
        // TODO IR-1183 add returned message check 29.03.2018
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

  /**
   * Make expected message for test with wrong key size.
   * @param public_key - wrongly sized key
   * @return message expected from test
   */
  std::string makeMessageWrongKeySize(const std::string &public_key) {
    return (boost::format("Public key has wrong size, passed size: %d")
            % public_key.size())
        .str();
  }

  /// Create test case with field assignment initialization function
  template <typename F, typename V>
  FieldTestCase makeTestCase(const std::string &case_name,
                             F field,
                             const V &value,
                             bool valid,
                             const std::string &message) {
    return {
        case_name, [&, field, value] { this->*field = value; }, valid, message};
  }

  /// Create valid case with "valid" name, and empty message
  template <typename F, typename V>
  FieldTestCase makeValidCase(F field, const V &value) {
    return makeTestCase("valid", field, value, true, "");
  }

  /// Create invalid case with default message
  template <typename F>
  FieldTestCase makeInvalidCase(const std::string &case_name,
                                const std::string &field_name,
                                F field,
                                const std::string &value) {
    return makeTestCase(case_name, field, value, false, "");
  }

  /// Generate test cases for id types with name, separator, and domain
  template <typename F>
  std::vector<FieldTestCase> idTestCases(const std::string &field_name,
                                         F field,
                                         char separator) {
    auto f = [&](const auto &s) {
      return (boost::format(s) % separator).str();
    };

    auto c = [&](const auto &n, const auto &v) {
      return this->makeInvalidCase(n, field_name, field, v);
    };

    return {makeValidCase(field, f("name%cdomain")),
            c("domain_start_with_digit", f("abs%c3domain")),
            c("empty_string", ""),
            c("illegal_char", f("ab--s%cdo--main")),
            c(f("missing_%c"), "absdomain"),
            c("missing_name", f("%cdomain"))};
  }

  std::vector<FieldTestCase> account_id_test_cases =
      idTestCases("account_id", &FieldValidatorTest::account_id, '@');

  std::vector<FieldTestCase> asset_id_test_cases =
      idTestCases("asset_id", &FieldValidatorTest::asset_id, '#');

  std::vector<FieldTestCase> amount_test_cases{
      {"valid_amount",
       [&] { amount.mutable_value()->set_fourth(100); },
       true,
       ""},
      {"zero_amount",
       [&] { amount.mutable_value()->set_fourth(0); },
       false,
       "Amount must be greater than 0, passed value: 0"}};

  /**
   * Make test case for invalid peer address.
   * @param case_name - test case name
   * @param address - peer address
   * @param pubkey - peer public key
   * @return test case for invalid peer address
   */
  FieldTestCase makeInvalidPeerAddressTestCase(const std::string &case_name,
                                               const std::string &address,
                                               const std::string &pubkey) {
    return {case_name,
            [&, address, pubkey] {
              this->peer.set_address(address);
              this->peer.set_peer_key(pubkey);
            },
            false,
            ""};
  }

  /**
   * Make test case for valid peer address.
   * @param case_name - test case name
   * @param address - peer address
   * @param pubkey - peer public key
   * @return test case for valid peer address
   */
  FieldTestCase makeValidPeerAddressTestCase(const std::string &case_name,
                                             const std::string &address,
                                             const std::string &pubkey) {
    return {case_name,
            [&, address, pubkey] {
              this->peer.set_address(address);
              this->peer.set_peer_key(pubkey);
            },
            true,
            ""};
  }

  /**
   * Make test case for invalid peer public key.
   * @param case_name - test case name
   * @param address - peer address
   * @param pubkey - peer public key
   * @return test case for invalid peer public key
   */
  FieldTestCase makeInvalidPeerPubkeyTestCase(const std::string &case_name,
                                              const std::string &address,
                                              const std::string &pubkey) {
    return {case_name,
            [&, address, pubkey] {
              this->peer.set_address(address);
              this->peer.set_peer_key(pubkey);
            },
            false,
            makeMessageWrongKeySize(pubkey)};
  }

  /**
   * Make test case for invalid peer public key.
   * @param case_name - test case name
   * @param address - peer address
   * @param pubkey - peer public key
   * @return test case for invalid peer public key
   */
  FieldTestCase invalidPublicKeyTestCase(const std::string &case_name,
                                         const std::string &public_key) {
    return makeTestCase(case_name,
                        &FieldValidatorTest::public_key,
                        public_key,
                        false,
                        makeMessageWrongKeySize(public_key));
  }

  std::vector<FieldTestCase> public_key_test_cases{
      makeValidCase(&FieldValidatorTest::public_key, std::string(32, '0')),
      invalidPublicKeyTestCase("invalid_key_length", std::string(64, '0')),
      invalidPublicKeyTestCase("empty_string", "")};

  std::vector<FieldTestCase> peer_test_cases{
      // clang-format off
      // ip addresses
      makeValidPeerAddressTestCase("zeros_ip","0.0.0.0:0", std::string(32, '0')),
      makeValidPeerAddressTestCase("max_ip", "255.255.255.255:65535", std::string(32, '0')),
      makeValidPeerAddressTestCase("common_ip","192.168.0.1:8080", std::string(32, '0')),

      makeInvalidPeerAddressTestCase("invalid_peer_address", "182.13.35.1:3040xx", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("invalid symbol in ip", "-0.0.0.0:0", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("too big number in ip", "256.256.256.255:65535", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("too big port", "192.168.0.1:65536", std::string(32, '0')),

      // hostname
      makeValidPeerAddressTestCase("valid hostname with port", "abc.efg:0", std::string(32, '0')),
      makeValidPeerAddressTestCase("valid hostname with max port", "abc.efg.hij:65535", std::string(32, '0')),
      makeValidPeerAddressTestCase("hostname with hyphen", "a-hyphen.ru-i:8080", std::string(32, '0')),
      makeValidPeerAddressTestCase("common hostname with port", "altplus.com.jp:80", std::string(32, '0')),
      makeValidPeerAddressTestCase("max label length in hostname with port", "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad:8080", std::string(32, '0')),
      makeValidPeerAddressTestCase("max domain name length in hostname with port",
                                   "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
                                   "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
                                   "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
                                   "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad:256", std::string(32, '0')),


      makeInvalidPeerAddressTestCase("hostname starting with nonletter", "9.start.with.non.letter:0", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("hostname starting with dash", "-startWithDash:65535", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("hostname starting with at", "@.is.not.allowed:8080", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("hostname with space", "no space is allowed:80", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("hostname with too big port", "too.big.port:65536", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("hostname with not allowed character", "some\u2063host:123", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("hostname ending with hyphen", "endWith-:909", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("hostname with too large label", "aLabelMustNotExceeds63charactersALabelMustNotExceeds63characters:9090", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("hostname with more than 256 character domain",
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
      "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPadP:256", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("empty address", "", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("empty domain", ":6565", std::string(32, '0')),
      makeInvalidPeerAddressTestCase("empty domain two : symbols", "::6565:", std::string(32, '0')),

      // invalid pubkey
      makeInvalidPeerPubkeyTestCase("invalid_peer_pubkey_length",
                                    "182.13.35.1:3040",
                                    std::string(64, '0')),
      makeInvalidPeerPubkeyTestCase(
          "invalid_peer_pubkey_empty", "182.13.35.1:3040", "")
      // clang-format on
  };

  /// Generate test cases for name types (account_name, asset_name, role_id)
  template <typename F>
  std::vector<FieldTestCase> nameTestCases(const std::string &field_name,
                                           F field) {
    return {makeTestCase("valid_name", field, "admin", true, ""),
            makeInvalidCase("empty_string", field_name, field, ""),
            makeInvalidCase("illegal_characters", field_name, field, "-math-"),
            makeInvalidCase("name_too_long",
                            field_name,
                            field,
                            "long_long_long_long_long_long_name")};
  }

  std::vector<FieldTestCase> role_name_test_cases =
      nameTestCases("role_id", &FieldValidatorTest::role_name);

  std::vector<FieldTestCase> account_name_test_cases =
      nameTestCases("account_name", &FieldValidatorTest::account_name);

  std::vector<FieldTestCase> asset_name_test_cases =
      nameTestCases("asset_name", &FieldValidatorTest::asset_name);

  /// domain_id
  std::vector<FieldTestCase> domainIdTestCases() {
    auto true_case = [&](const auto &name, const auto &case_value) {
      return this->makeTestCase(
          name, &FieldValidatorTest::domain_id, case_value, true, "");
    };
    auto false_case = [&](const auto &name, const auto &case_value) {
      return this->makeTestCase(
          name, &FieldValidatorTest::domain_id, case_value, false, "");
    };
    return {
        // clang-format off
      true_case("one_letter", "a"),
      true_case("two_letter", "ab"),
      true_case("period_separated", "abc.efg"),
      true_case("multiple_periods_separated", "abc.efg.hij"),
      true_case("with_numbers", "u9EEA432F"),
      true_case("with_hyphen", "a-hyphen"),
      true_case("with_63_character", "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad"),
      true_case("ending_with_digit","endWith0"),
      true_case("max_long_domain",
                 "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
                 "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
                 "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
                 "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad"),

      false_case("space", " "),
      false_case("start_with_digit", "9start.with.non.letter"),
      false_case("start_with_dash", "-startWithDash"),
      false_case("with_@", "@.is.not.allowed"),
      false_case("with_space","no space is allowed"),
      false_case("end_with_hyphen", "endWith-"),
      false_case("label_ending_with_hyphen","label.endedWith-.is.not.allowed"),
      false_case("too_long_label","aLabelMustNotExceeds63charactersALabelMustNotExceeds63characters"),
      false_case("too_long_domain",
                 "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
                 "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
                 "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPad."
                 "maxLabelLengthIs63paddingPaddingPaddingPaddingPaddingPaddingPadP")
        // clang-format on
    };
  }
  std::vector<FieldTestCase> domain_id_test_cases = domainIdTestCases();

  std::vector<FieldTestCase> permissions_test_cases{
      makeValidCase(&FieldValidatorTest::role_permission,
                    iroha::protocol::RolePermission::can_append_role)};

  std::vector<FieldTestCase> counter_test_cases{
      makeValidCase(&FieldValidatorTest::counter, 5),
      makeTestCase("zero_counter",
                   &FieldValidatorTest::counter,
                   0,
                   false,
                   "Counter should be > 0, passed value: 0")};
  std::vector<FieldTestCase> created_time_test_cases{
      makeValidCase(&FieldValidatorTest::created_time, iroha::time::now()),
      makeValidCase(&FieldValidatorTest::created_time,
                    iroha::time::now() + nearest_future),
      makeTestCase(
          "invalid due to far future",
          &FieldValidatorTest::created_time,
          iroha::time::now() + far_future,
          false,
          "bad timestamp: sent from future, timestamp: [0-9]+, now: [0-9]+")};

  std::vector<FieldTestCase> detail_key_test_cases{
      makeValidCase(&FieldValidatorTest::detail_key, "happy"),
      makeInvalidCase(
          "empty_string", "key", &FieldValidatorTest::detail_key, ""),
      makeInvalidCase(
          "illegal_char", "key", &FieldValidatorTest::detail_key, "hi-there")};

  std::vector<FieldTestCase> detail_value_test_cases{
      makeValidCase(&FieldValidatorTest::detail_value, "valid value"),
      makeValidCase(&FieldValidatorTest::detail_value, std::string(4096, '0')),
      makeValidCase(&FieldValidatorTest::detail_value, ""),
      makeInvalidCase(
          "long_value",
          "value",
          &FieldValidatorTest::detail_value,
          // 5 Mb, value greater than can put into one setAccountDetail
          std::string(5 * 1024 * 1024, '0'))};

  std::vector<FieldTestCase> description_test_cases{
      makeValidCase(&FieldValidatorTest::description, "valid description"),
      makeValidCase(&FieldValidatorTest::description, ""),
      makeValidCase(&FieldValidatorTest::description, std::string(64, 0)),
      makeInvalidCase("long_description",
                      "value",
                      &FieldValidatorTest::description,
                      std::string(65, '0'))};

  std::vector<FieldTestCase> quorum_test_cases{
      makeValidCase(&FieldValidatorTest::quorum, 1),
      makeValidCase(&FieldValidatorTest::quorum, 128),
      makeTestCase(
          "too big quorum size", &FieldValidatorTest::quorum, 129, false, "")};

  std::vector<FieldTestCase> permissionTestCases() {
    auto valid_cases =
        shared_model::permissions::role_perm_group
        | boost::adaptors::transformed([&](const auto &permission) {
            return this->makeTestCase(permission,
                                      &FieldValidatorTest::permission,
                                      permission,
                                      true,
                                      "");
          });
    std::vector<FieldTestCase> invalid_cases = {
        makeInvalidCase("non existing permission",
                        "permission",
                        &FieldValidatorTest::permission,
                        "non_existing_permission")};
    // merge valid and invalid cases
    std::vector<FieldTestCase> all_cases;
    all_cases.insert(all_cases.begin(), valid_cases.begin(), valid_cases.end());
    all_cases.insert(
        all_cases.begin(), invalid_cases.begin(), invalid_cases.end());
    return all_cases;
  }
  std::vector<FieldTestCase> permission_test_cases = permissionTestCases();

  /**************************************************************************/

  /// Create no-operation validator
  std::pair<std::string, FieldTest> makeNullValidator(
      const std::string &field_name) {
    return {field_name, {}};
  }

  /// Create validator with given field transformation
  template <typename F, typename V, typename T>
  std::pair<std::string, FieldTest> makeTransformValidator(
      const std::string &field_name,
      F field,
      V value,
      T transform,
      const std::vector<FieldTestCase> &cases) {
    return {field_name,
            {[&, field, value] {
               validation::ReasonsGroupType reason;
               (field_validator.*field)(reason, transform(this->*value));
               return reason;
             },
             cases}};
  }

  /// Create validator with identity transformation
  template <typename F, typename V>
  std::pair<std::string, FieldTest> makeValidator(
      const std::string &field_name,
      F field,
      V value,
      const std::vector<FieldTestCase> &cases) {
    return makeTransformValidator(
        field_name, field, value, [](auto &&x) { return x; }, cases);
  }

  using FieldValidator = validation::FieldValidator;

  // register validation function and test cases
  std::unordered_map<std::string, FieldTest> field_validators{
      // Command fields
      makeValidator("asset_id",
                    &FieldValidator::validateAssetId,
                    &FieldValidatorTest::asset_id,
                    asset_id_test_cases),
      makeTransformValidator("amount",
                             &FieldValidator::validateAmount,
                             &FieldValidatorTest::amount,
                             [](auto &&x) { return proto::Amount(x); },
                             amount_test_cases),
      makeTransformValidator("peer",
                             &FieldValidator::validatePeer,
                             &FieldValidatorTest::peer,
                             [](auto &&x) { return proto::Peer(x); },
                             peer_test_cases),
      makeValidator("account_name",
                    &FieldValidator::validateAccountName,
                    &FieldValidatorTest::account_name,
                    account_name_test_cases),
      makeValidator("domain_id",
                    &FieldValidator::validateDomainId,
                    &FieldValidatorTest::domain_id,
                    domain_id_test_cases),
      makeValidator("asset_name",
                    &FieldValidator::validateAssetName,
                    &FieldValidatorTest::asset_name,
                    asset_name_test_cases),
      makeTransformValidator("permissions",
                             &FieldValidator::validatePermissions,
                             &FieldValidatorTest::role_permission,
                             [](auto &&x) {
                               return interface::CreateRole::PermissionsType{
                                   iroha::protocol::RolePermission_Name(x)};
                             },
                             permissions_test_cases),
      makeValidator("created_time",
                    &FieldValidator::validateCreatedTime,
                    &FieldValidatorTest::created_time,
                    created_time_test_cases),
      makeValidator("description",
                    &FieldValidator::validateDescription,
                    &FieldValidatorTest::description,
                    description_test_cases)};
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
  iroha::protocol::Transaction proto_tx;
  proto_tx.add_signatures();  // at least one signature in message

  // iterate over all fields in transaction
  iterateContainer(
      [] { return iroha::protocol::Transaction::descriptor(); },
      [&](auto field) {
        return field->is_repeated()
            ? proto_tx.GetReflection()->MutableRepeatedMessage(
                  &proto_tx, field, 0)
            : proto_tx.GetReflection()->MutableMessage(&proto_tx, field);
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
      [] {});
}

/**
 * @given field values from test cases
 * @when field validator is invoked on query container's fields
 * @then field validator correctly rejects invalid values, and provides
 * meaningful message
 */
TEST_F(FieldValidatorTest, QueryContainerFieldsValidation) {
  iroha::protocol::Query query_tx;

  // iterate over all fields in transaction
  iterateContainer(
      [] { return iroha::protocol::Query::descriptor(); },
      [&](auto field) {
        return query_tx.GetReflection()->MutableMessage(&query_tx, field);
      },
      [this](auto field, auto) {
        // Skip oneof types
        if (field->containing_oneof()
            == iroha::protocol::Query::Payload::descriptor()->FindOneofByName(
                   "query")) {
          return;
        }
        this->runTestCases(field);
      },
      [] {});
}
