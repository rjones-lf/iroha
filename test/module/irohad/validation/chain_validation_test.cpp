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

#include "builders/common_objects/signature_builder.hpp"
#include "builders/protobuf/builder_templates/block_template.hpp"
#include "builders/protobuf/common_objects/proto_signature_builder.hpp"
#include "module/irohad/ametsuchi/ametsuchi_mocks.hpp"
#include "module/irohad/model/model_mocks.hpp"
#include "validation/impl/chain_validator_impl.hpp"

// TODO: 14-02-2018 Alexey Chernyshov remove after relocation to shared_model
#include "backend/protobuf/from_old_model.hpp"

using namespace iroha;
using namespace iroha::model;
using namespace iroha::validation;
using namespace iroha::ametsuchi;

using ::testing::A;
using ::testing::ByRef;
using ::testing::InvokeArgument;
using ::testing::Return;
using ::testing::_;

class ChainValidationTest : public ::testing::Test {
 public:
  void SetUp() override {
    validator = std::make_shared<ChainValidatorImpl>();
    storage = std::make_shared<MockMutableStorage>();
    query = std::make_shared<MockWsvQuery>();

    // TODO: 14-02-2018 Alexey Chernyshov remove after replacement
    // with shared_model https://soramitsu.atlassian.net/browse/IR-903
    std::copy(
        public_key.blob().begin(), public_key.blob().end(), peer.pubkey.data());
    peers = std::vector<Peer>{peer};
  }

  /**
   * Get block builder to build blocks for tests
   * @return block builder
   */
  auto getBlockBuilder() const {
    constexpr auto kTotal = (1 << 5) - 1;
    return shared_model::proto::TemplateBlockBuilder<
               kTotal,
               shared_model::validation::DefaultBlockValidator,
               shared_model::proto::Block>()
        .txNumber(0)
        .height(1)
        .prevHash(hash)
        .createdTime(iroha::time::now());
  }

  /**
   * Add signature to the block.
   */
  void addSignature(shared_model::interface::Block &block,
                    shared_model::interface::types::PubkeyType &pubkey) {
    shared_model::builder::SignatureBuilder<
        shared_model::proto::SignatureBuilder,
        shared_model::validation::FieldValidator>
        builder;
    auto signature = builder.publicKey(pubkey).build();

    signature.match(
        [&](shared_model::builder::BuilderResult<
            shared_model::interface::Signature>::ValueType &sig) {
          block.addSignature(sig.value);
        },
        [](shared_model::builder::BuilderResult<
            shared_model::interface::Signature>::ErrorType &e) {
          FAIL() << *e.error;
        });
  }

  shared_model::interface::types::PubkeyType public_key =
      shared_model::interface::types::PubkeyType(std::string(32, '2'));

  // TODO: 14-02-2018 Alexey Chernyshov remove after replacement
  // with shared_model https://soramitsu.atlassian.net/browse/IR-903
  Peer peer;
  std::vector<Peer> peers;

  shared_model::crypto::Hash hash =
      shared_model::crypto::Hash(std::string(32, '0'));

  std::shared_ptr<ChainValidatorImpl> validator;
  std::shared_ptr<MockMutableStorage> storage;
  std::shared_ptr<MockWsvQuery> query;
};

TEST_F(ChainValidationTest, ValidCase) {
  // Valid previous hash, has supermajority, correct peers subset => valid
  auto block = getBlockBuilder().build();
  auto tmp = std::make_shared<shared_model::proto::Block>(block.getTransport());
  auto bl = std::dynamic_pointer_cast<shared_model::interface::Block>(tmp);
  addSignature(block, public_key);

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(bl, _))
      .WillOnce(InvokeArgument<1>(bl, ByRef(*query), ByRef(hash)));

  ASSERT_TRUE(validator->validateBlock(block, *storage));
}

TEST_F(ChainValidationTest, FailWhenDifferentPrevHash) {
  // Invalid previous hash, has supermajority, correct peers subset => invalid
  auto block = getBlockBuilder().build();
  auto tmp = std::make_shared<shared_model::proto::Block>(block.getTransport());
  auto bl = std::dynamic_pointer_cast<shared_model::interface::Block>(tmp);

  addSignature(block, public_key);

  shared_model::crypto::Hash another_hash =
      shared_model::crypto::Hash(std::string(32, '1'));

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(bl, _))
      .WillOnce(InvokeArgument<1>(bl, ByRef(*query), ByRef(another_hash)));

  ASSERT_FALSE(validator->validateBlock(block, *storage));
}

TEST_F(ChainValidationTest, FailWhenNoSupermajority) {
  // Valid previous hash, no supermajority, correct peers subset => invalid
  auto block = getBlockBuilder().build();
  auto tmp = std::make_shared<shared_model::proto::Block>(block.getTransport());
  auto bl = std::dynamic_pointer_cast<shared_model::interface::Block>(tmp);

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(bl, _))
      .WillOnce(InvokeArgument<1>(bl, ByRef(*query), ByRef(hash)));

  ASSERT_FALSE(validator->validateBlock(block, *storage));
}

TEST_F(ChainValidationTest, FailWhenBadPeer) {
  // Valid previous hash, has supermajority, incorrect peers subset => invalid
  shared_model::interface::types::PubkeyType wrong_public_key =
      shared_model::interface::types::PubkeyType(std::string(32, '1'));
  auto block = getBlockBuilder().build();
  auto tmp = std::make_shared<shared_model::proto::Block>(block.getTransport());
  auto bl = std::dynamic_pointer_cast<shared_model::interface::Block>(tmp);

  addSignature(block, wrong_public_key);

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  EXPECT_CALL(*storage, apply(bl, _))
      .WillOnce(InvokeArgument<1>(bl, ByRef(*query), ByRef(hash)));

  ASSERT_FALSE(validator->validateBlock(block, *storage));
}

TEST_F(ChainValidationTest, ValidWhenValidateChainFromOnePeer) {
  // Valid previous hash, has supermajority, correct peers subset => valid
  auto block = getBlockBuilder().build();
  auto tmp = std::make_shared<shared_model::proto::Block>(block.getTransport());
  auto bl = std::dynamic_pointer_cast<shared_model::interface::Block>(tmp);

  addSignature(block, public_key);

  EXPECT_CALL(*query, getPeers()).WillOnce(Return(peers));

  // TODO: 14-02-2018 Alexey Chernyshov add argument after replacement
  // with shared_model https://soramitsu.atlassian.net/browse/IR-903
  iroha::model::Block old_block;
  old_block.sigs.emplace_back();
  old_block.sigs.back().pubkey = peer.pubkey;
  old_block.prev_hash.fill(0);
  auto block_observable = rxcpp::observable<>::just(old_block);

  EXPECT_CALL(*storage, apply(/* TODO block */ _, _))
      .WillOnce(InvokeArgument<1>(bl, ByRef(*query), ByRef(hash)));

  ASSERT_TRUE(validator->validateChain(block_observable, *storage));
}
