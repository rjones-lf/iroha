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

#ifndef IROHA_VALIDATORS_FIXTURE_HPP
#define IROHA_VALIDATORS_FIXTURE_HPP

#include <gtest/gtest.h>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/irange.hpp>

#include "datetime/time.hpp"
#include "primitive.pb.h"

class ValidatorsTest : public ::testing::Test {
 public:
  ValidatorsTest() {
    valid_amount.set_precision(2);
    valid_amount.mutable_value()->set_fourth(1000);

    // Generate protobuf reflection setter for given type and value
    auto setField = [&](auto setter) {
      return [setter](const auto &value) {
        return [setter, &value](auto refl, auto msg, auto field) {
          (refl->*setter)(msg, field, value);
        };
      };
    };

    auto setString = setField(&google::protobuf::Reflection::SetString);
    auto addString = setField(&google::protobuf::Reflection::AddString);
    auto setUInt32 = setField(&google::protobuf::Reflection::SetUInt32);
    auto addEnum = setField(&google::protobuf::Reflection::AddEnumValue);
    auto setEnum = setField(&google::protobuf::Reflection::SetEnumValue);

    for (const auto &id : {"account_id", "src_account_id", "dest_account_id"}) {
      field_setters[id] = setString(valid_account_id);
    }
    for (const auto &id : {"peer_key", "public_key", "main_pubkey"}) {
      field_setters[id] = setString(valid_public_key);
    }
    for (const auto &id : {"role_name", "default_role", "role_id"}) {
      field_setters[id] = setString(valid_role_name);
    }
    field_setters["asset_id"] = setString(valid_asset_id);
    field_setters["address"] = setString(valid_address_localhost);
    field_setters["account_name"] = setString(valid_account_name);
    field_setters["domain_id"] = setString(valid_domain_id);
    field_setters["asset_name"] = setString(valid_asset_name);
    field_setters["precision"] = setUInt32(valid_precision);
    field_setters["permissions"] = addEnum(valid_role_permission);
    field_setters["permission"] = setEnum(valid_grantable_permission);
    field_setters["key"] = setString(valid_detail_key);
    field_setters["detail"] = setString(valid_detail_key);
    field_setters["value"] = setString("");
    field_setters["tx_hashes"] = addString(valid_hash);
    field_setters["quorum"] = setUInt32(valid_quorum);
    field_setters["description"] = setString("");
    field_setters["amount"] = [&](auto refl, auto msg, auto field) {
      refl->MutableMessage(msg, field)->CopyFrom(valid_amount);
    };
  }

  /**
   * Iterate the container (transaction or query), generating concrete subtypes
   * and doing operation on concrete subtype fields. Call validator after each
   * subtype
   * @tparam DescGen oneof descriptor generator type
   * @tparam ConcreteGen concrete subtype generator type
   * @tparam FieldOp field operation type
   * @tparam Validator validator type
   * @param desc_gen descriptor generator callable object
   * @param concrete_gen concrete subtype generator callable object
   * @param field_op field operation callable object
   * @param validator validator callable object
   */
  template <typename DescGen,
            typename ConcreteGen,
            typename FieldOp,
            typename Validator>
  void iterateContainer(DescGen &&desc_gen,
                        ConcreteGen &&concrete_gen,
                        FieldOp &&field_op,
                        Validator &&validator) {
    auto desc = desc_gen();
    // Get field descriptor for concrete type
    const auto &range = boost::irange(0, desc->field_count())
        | boost::adaptors::transformed([&](auto i) { return desc->field(i); });
    // Iterate through all concrete types
    boost::for_each(range, [&](auto field) {
      auto concrete = concrete_gen(field);

      // Iterate through all fields of concrete type
      auto concrete_desc = concrete->GetDescriptor();
      // Get field descriptor for concrete type field
      const auto &range = boost::irange(0, concrete_desc->field_count())
          | boost::adaptors::transformed(
                              [&](auto i) { return concrete_desc->field(i); });
      boost::for_each(range, [&](auto field) { field_op(field, concrete); });
      validator();
    });
  }

 protected:
  void SetUp() override {
    valid_created_time = iroha::time::now();
  }

  size_t public_key_size = 32, hash_size = 32;
  uint64_t valid_counter = 1048576;
  std::string valid_account_id = "account@domain", valid_asset_name = "asset",
              valid_asset_id = "asset#domain",
              valid_address_localhost = "localhost:65535",
              valid_address_ipv4 = "192.168.255.1:8080",
              valid_address_hostname = "google.ru:8080",
              valid_role_name = "user", valid_account_name = "admin",
              valid_domain_id = "ru", valid_detail_key = "key",
              valid_public_key = std::string(public_key_size, '0'),
              valid_hash = std::string(public_key_size, '0');
  iroha::protocol::RolePermission valid_role_permission =
      iroha::protocol::RolePermission::can_append_role;
  iroha::protocol::GrantablePermission valid_grantable_permission =
      iroha::protocol::GrantablePermission::can_add_my_signatory;
  uint8_t valid_quorum = 2;
  uint8_t valid_precision = 42;
  iroha::protocol::Amount valid_amount;
  decltype(iroha::time::now()) valid_created_time;

  // List all used fields in commands
  std::unordered_map<
      std::string,
      std::function<void(const google::protobuf::Reflection *,
                         google::protobuf::Message *,
                         const google::protobuf::FieldDescriptor *)>>
      field_setters;
};

#endif  // IROHA_VALIDATORS_FIXTURE_HPP
