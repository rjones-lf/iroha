/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "consensus/yac/impl/supermajority_checker_impl.hpp"
#include "module/irohad/ametsuchi/ametsuchi_fixture.hpp"
#include "validation/impl/chain_validator_impl.hpp"

#include "ametsuchi/mutable_storage.hpp"
#include "builders/protobuf/transaction.hpp"
#include "cryptography/crypto_provider/crypto_defaults.hpp"
#include "cryptography/default_hash_provider.hpp"
#include "cryptography/keypair.hpp"
#include "module/shared_model/builders/protobuf/block.hpp"

namespace iroha {

  class ChainValidatorStorageTest : public ametsuchi::AmetsuchiTest {
   public:
    void SetUp() override {
      ametsuchi::AmetsuchiTest::SetUp();
      validator = std::make_shared<validation::ChainValidatorImpl>(
          std::make_shared<consensus::yac::SupermajorityCheckerImpl>());

      for (size_t i = 0; i < 5; ++i) {
        keys.push_back(shared_model::crypto::DefaultCryptoAlgorithmType::
                           generateKeypair());
      }
    }

    /// Create transaction builder with filled account id, created time, quorum
    auto baseTx() {
      return shared_model::proto::TransactionBuilder()
          .creatorAccountId("admin@test")
          .createdTime(iroha::time::now())
          .quorum(1);
    }

    /// Complete builder by adding a signature and return a signed transaction
    template <typename Builder>
    auto completeTx(Builder builder) {
      return builder.build().signAndAddSignature(keys.at(0)).finish();
    }

    /// Generate a dummy transaction with create role command
    auto dummyTx(std::size_t i) {
      return completeTx(baseTx().createRole("role" + std::to_string(i), {}));
    }

    /// Create block unsigned wrapper with given transactions, height, prev hash
    auto baseBlock(std::vector<shared_model::proto::Transaction> transactions,
                   shared_model::interface::types::HeightType height,
                   shared_model::interface::types::HashType prev_hash) {
      return shared_model::proto::BlockBuilder()
          .transactions(transactions)
          .height(height)
          .prevHash(prev_hash)
          .createdTime(iroha::time::now())
          .build();
    }

    /// Complete wrapper and return a signed object
    template <typename Wrapper>
    auto completeBlock(Wrapper &&wrapper) {
      return std::forward<Wrapper>(wrapper).finish();
    }

    /// Create mutable storage from initialized storage
    auto createMutableStorage() {
      return boost::get<
                 expected::Value<std::unique_ptr<ametsuchi::MutableStorage>>>(
                 storage->createMutableStorage())
          .value;
    }

    /// Create first block with 4 peers, apply it to storage and return it
    auto generateAndApplyFirstBlock() {
      auto tx =
          completeTx(baseTx()
                         .addPeer("0.0.0.0:50541", keys.at(0).publicKey())
                         .addPeer("0.0.0.0:50542", keys.at(1).publicKey())
                         .addPeer("0.0.0.0:50543", keys.at(2).publicKey())
                         .addPeer("0.0.0.0:50544", keys.at(3).publicKey()));

      auto block = completeBlock(
          baseBlock({tx},
                    1,
                    shared_model::crypto::DefaultHashProvider::makeHash(
                        shared_model::crypto::Blob("")))
              .signAndAddSignature(keys.at(0)));

      auto ms = createMutableStorage();

      ms->apply(block);
      storage->commit(std::move(ms));

      return block;
    }

    /// Create an observable from chain and return its validation status
    auto createAndValidateChain(
        std::vector<std::shared_ptr<shared_model::interface::Block>> chain) {
      auto ms = createMutableStorage();
      return validator->validateAndApply(rxcpp::observable<>::iterate(chain), *ms);
    }

    std::shared_ptr<validation::ChainValidatorImpl> validator;
    std::vector<shared_model::crypto::Keypair> keys;
  };

  /**
   * @given initialized storage
   * block 1 - initial block with 4 peers
   * block 2 - new peer added. signed by supermajority of ledger peers
   * block 3 - signed by supermajority of ledger peers, contains signature of
   * new peer
   * @when blocks 2 and 3 are validated
   * @then result is successful
   */
  TEST_F(ChainValidatorStorageTest, PeerAdded) {
    auto block1 = generateAndApplyFirstBlock();

    auto add_peer =
        completeTx(baseTx().addPeer("0.0.0.0:50545", keys.at(4).publicKey()));
    auto block2 = completeBlock(baseBlock({add_peer}, 2, block1.hash())
                                    .signAndAddSignature(keys.at(0))
                                    .signAndAddSignature(keys.at(1))
                                    .signAndAddSignature(keys.at(2)));

    auto block3 = completeBlock(baseBlock({dummyTx(3)}, 3, block2.hash())
                                    .signAndAddSignature(keys.at(0))
                                    .signAndAddSignature(keys.at(1))
                                    .signAndAddSignature(keys.at(2))
                                    .signAndAddSignature(keys.at(3))
                                    .signAndAddSignature(keys.at(4)));

    ASSERT_TRUE(createAndValidateChain({clone(block2), clone(block3)}));
  }

  /**
   * @given initialized storage with 4 peers
   * block 1 - initial block with 4 peers
   * block 2 - signed by supermajority of ledger peers
   * block 3 - signed by supermajority of ledger peers
   * @when blocks 2 and 3 are validated
   * @then result is successful
   */
  TEST_F(ChainValidatorStorageTest, NoPeerAdded) {
    auto block1 = generateAndApplyFirstBlock();

    auto block2 = completeBlock(baseBlock({dummyTx(2)}, 2, block1.hash())
                                    .signAndAddSignature(keys.at(0))
                                    .signAndAddSignature(keys.at(1))
                                    .signAndAddSignature(keys.at(2)));

    auto block3 = completeBlock(baseBlock({dummyTx(3)}, 3, block2.hash())
                                    .signAndAddSignature(keys.at(0))
                                    .signAndAddSignature(keys.at(1))
                                    .signAndAddSignature(keys.at(2))
                                    .signAndAddSignature(keys.at(3)));

    ASSERT_TRUE(createAndValidateChain({clone(block2), clone(block3)}));
  }

  /**
   * @given initialized storage
   * block 1 - initial block with 4 peers
   * block 2 - invalid previous hash, signed by supermajority
   * @when block 2 is validated
   * @then result is not successful
   */
  TEST_F(ChainValidatorStorageTest, InvalidHash) {
    auto block1 = generateAndApplyFirstBlock();

    auto block2 = completeBlock(
        baseBlock({dummyTx(2)},
                  2,
                  shared_model::crypto::DefaultHashProvider::makeHash(
                      shared_model::crypto::Blob("bad_hash")))
            .signAndAddSignature(keys.at(0))
            .signAndAddSignature(keys.at(1))
            .signAndAddSignature(keys.at(2)));

    ASSERT_FALSE(createAndValidateChain({clone(block2)}));
  }

  /**
   * @given initialized storage
   * block 1 - initial block with 4 peers
   * block 2 - signed by only 2 out of 4 peers, no supermajority
   * @when block 2 is validated
   * @then result is not successful
   */
  TEST_F(ChainValidatorStorageTest, NoSupermajority) {
    auto block1 = generateAndApplyFirstBlock();

    auto block2 = completeBlock(baseBlock({dummyTx(2)}, 2, block1.hash())
                                    .signAndAddSignature(keys.at(0))
                                    .signAndAddSignature(keys.at(1)));

    ASSERT_FALSE(createAndValidateChain({clone(block2)}));
  }

}  // namespace iroha
