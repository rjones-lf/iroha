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

#include "model/model_crypto_provider_impl.hpp"
#include "model/converters/pb_block_factory.hpp"
#include "model/converters/pb_common.hpp"
#include "model/converters/pb_query_factory.hpp"
#include "model/converters/pb_transaction_factory.hpp"

namespace iroha {
  namespace model {

    const static model::converters::PbTransactionFactory transaction_factory;
    const static model::converters::PbBlockFactory block_factory;
    const static model::converters::PbQueryFactory query_factory;

    ModelCryptoProviderImpl::ModelCryptoProviderImpl(const keypair_t &keypair)
        : keypair_(keypair) {}

    bool ModelCryptoProviderImpl::verify(const Transaction &tx) const {
      auto pb_tx = transaction_factory.serialize(tx);
      
      return std::all_of(
          tx.signatures.begin(), tx.signatures.end(), iroha::verify(pb_tx));
    }

    bool ModelCryptoProviderImpl::verify(
        std::shared_ptr<const Query> query) const {
      auto pb_query = *query_factory.serialize(query);

      return iroha::verify(pb_query)(query->signature);
    }

    bool ModelCryptoProviderImpl::verify(const Block &block) const {
      auto pb_block = block_factory.serialize(block);

      return std::all_of(
          block.sigs.begin(), block.sigs.end(), iroha::verify(pb_block));
    }

    Block ModelCryptoProviderImpl::sign(const Block &block) const {
      auto pb_block = block_factory.serialize(block);

      auto signature = iroha::sign(pb_block, keypair_);

      auto signed_block = block;
      signed_block.sigs.push_back(Signature{signature, keypair_.pubkey});
      return signed_block;
    }

    Transaction ModelCryptoProviderImpl::sign(
        const Transaction &transaction) const {
      auto pb_transaction = transaction_factory.serialize(transaction);

      auto signature = iroha::sign(pb_transaction, keypair_);

      auto signed_transaction = transaction;
      signed_transaction.signatures.push_back(
          Signature{signature, keypair_.pubkey});
      return signed_transaction;
    }
  }
}
