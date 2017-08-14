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

#include <iostream>
#include <model/model_hash_provider_impl.hpp>
#include <model/queries/get_account.hpp>
#include "common/types.hpp"
#include "model/queries/get_account_assets.hpp"
#include "model/queries/get_signatories.hpp"
#include "model/queries/get_transactions.hpp"

namespace iroha {
  namespace model {

    iroha::hash256_t HashProviderImpl::get_hash(const Proposal &proposal) {
      std::string concat_;  // string representation of the proposal, made of
      // proposals meta and body fields

      // Append body data
      for (auto tx : proposal.transactions) {
        // Append each transaction hash
        concat_ += get_hash(tx).to_string();
      }

      return sha3_256(concat_);
    }

    iroha::hash256_t HashProviderImpl::get_hash(const Block &block) {
      std::string concat_;

      // Append block height
      concat_ += std::to_string(block.height);

      // Append prev_hash
      concat_ += block.prev_hash.to_string();

      // Append txnumber
      concat_ += std::to_string(block.txs_number);

      // Append merkle root
      concat_ += block.merkle_root.to_string();

      // Append transactions data
      for (auto tx : block.transactions) {
        concat_ += get_hash(tx).to_string();
      }

      return sha3_256(concat_);
    }

    iroha::hash256_t HashProviderImpl::get_hash(const Transaction &tx) {
      // Resulting string for the hash

      // FIXME: sizeof(*command) is very bad, std::back_inserter with std::array is bad (check!)
      std::string concat_hash_commands_;
      for (auto command : tx.commands) {
        // convert command to blob and concat it to result string
        std::array<char, sizeof(*command)> command_blob;
        std::copy_n((char *)command.get(), sizeof(*command),
                    command_blob.begin());
        std::copy(command_blob.begin(), command_blob.end(),
                  std::back_inserter(concat_hash_commands_));
      }

      concat_hash_commands_ += tx.creator_account_id;

      // TODO: Decide if the header should be included
      /*
      for (auto sig : tx.signatures) {
        concat_ += sig.pubkey.to_string();
        concat_ += sig.signature.to_string();
      }
       */
      // Append tx counter
      concat_hash_commands_ += tx.tx_counter;

      std::vector<uint8_t> concat_hash_commands(concat_hash_commands_.begin(),
                                                concat_hash_commands_.end());

      auto concat_hash =
          sha3_256(concat_hash_commands.data(), concat_hash_commands.size());
      return concat_hash;
    }

    iroha::hash256_t HashProviderImpl::get_hash(
        std::shared_ptr<const Query> query) {
      std::string result_hash;
      if (instanceof <model::GetAccount>(query)) {
        auto cast = static_cast<const GetAccount &>(*query);
        result_hash += cast.account_id;
        result_hash += cast.creator_account_id;
      }
      if (instanceof <model::GetAccountAssets>(query)) {
        auto cast = static_cast<const GetAccountAssets &>(*query);
        result_hash += cast.account_id;
        result_hash += cast.asset_id;
        result_hash += cast.creator_account_id;
      }
      if (instanceof <model::GetSignatories>(query)) {
        auto cast = static_cast<const GetSignatories &>(*query);
        result_hash += cast.account_id;
        result_hash += cast.creator_account_id;
      }
      if (instanceof <model::GetAccountTransactions>(query)) {
        auto cast = static_cast<const GetAccountTransactions &>(*query);
        result_hash += cast.account_id;
        result_hash += cast.creator_account_id;
      }
      result_hash += query->query_counter;
      std::vector<uint8_t> concat_hash_commands(result_hash.begin(),
                                                result_hash.end());
      return sha3_256(concat_hash_commands.data(), concat_hash_commands.size());
    }

  }  // namespace model
}  // namespace iroha
