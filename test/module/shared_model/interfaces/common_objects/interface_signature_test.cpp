/**
 * Copyright Soramitsu Co., Ltd. 2018 All Rights Reserved.
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

#include "backend/protobuf/common_objects/signature.hpp"
#include "interfaces/common_objects/signature.hpp"
#include "queries.pb.h"

#include <gtest/gtest.h>
#include <memory>

using namespace shared_model;

/**
 * @given interface::Signature from proto::Signature
 * @when toString()
 * @then validate the string
 */
TEST(InterfaceSignatureTest, toString) {
  iroha::protocol::Signature protosig;
  protosig.set_pubkey(std::string(32, 0));
  protosig.set_signature(std::string(64, 0));
  std::unique_ptr<interface::Signature> sig =
      std::make_unique<proto::Signature>(protosig);
  auto expected = "Signature: [publicKey=" + std::string(64, '0')
      + ", signedData=" + std::string(128, '0') + ", ]";
  ASSERT_STREQ(expected.c_str(), sig->toString().c_str());
}

/**
 * @given interface::Signature from proto::Signature
 * @when makeOldModel()
 * @then validate valid old style signature
 */
TEST(InterfaceSignatureTest, makeOldModel) {
  iroha::protocol::Signature protosig;
  protosig.set_pubkey(std::string(32, 0));
  protosig.set_signature(std::string(64, 0));
  std::unique_ptr<interface::Signature> sig =
      std::make_unique<proto::Signature>(protosig);
  std::unique_ptr<iroha::model::Signature> old_sig(sig->makeOldModel());
  ASSERT_EQ(protosig.pubkey(),
            std::string(old_sig->pubkey.begin(), old_sig->pubkey.end()));
  ASSERT_EQ(protosig.signature(),
            std::string(old_sig->signature.begin(), old_sig->signature.end()));
}
