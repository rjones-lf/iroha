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

#ifndef IROHA_SIMPLE_BUILDER_HPP
#define IROHA_SIMPLE_BUILDER_HPP

#include "builders/protobuf/transaction.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "cryptography/public_key.hpp"

namespace shared_model {
  namespace bindings {
    /**
     * Wrapper class for transaction builder. Designed only for SWIG bindings,
     * don't use in other cases.
     */
    class ModelTransactionBuilder {
     private:
      template <int Sp>
      explicit ModelTransactionBuilder(const proto::TemplateTransactionBuilder<Sp> &o)
          : builder_(o) {}

     public:
      ModelTransactionBuilder() = default;

      /**
       * Sets id of account creator
       * @param account_id - account id
       * @return builder with account_id field appended
       */
      ModelTransactionBuilder creatorAccountId(
          const interface::types::AccountIdType &account_id);

      /**
       * Sets transaction counter field
       * @param tx_counter - transaction counter
       * @return builder with tx_counter field appended
       */
      ModelTransactionBuilder txCounter(uint64_t tx_counter);

      /**
       * Sets time of creation
       * @param created_time - time of creation
       * @return builder with created_time field appended
       */
      ModelTransactionBuilder createdTime(interface::types::TimestampType created_time);

      /**
       * Adds given quantity of given asset to account
       * @param account_id - account id
       * @param asset_id - asset id
       * @param amount - amount of asset to add
       * @return builder with asset quantity command appended
       */
      ModelTransactionBuilder addAssetQuantity(
          const interface::types::AccountIdType &account_id,
          const interface::types::AssetIdType &asset_id,
          const std::string &amount);

      /**
       * Adds new peer into ledger
       * @param address - peer address
       * @param peer_key - peer public key
       * @return builder with added peer command appended
       */
      ModelTransactionBuilder addPeer(const interface::types::AddressType &address,
                           const crypto::PublicKey &peer_key);

      /**
       * Adds new signatory
       * @param account_id - id of signatory's account
       * @param public_key - public key of signatory
       * @return builder with added signatory command appended
       */
      ModelTransactionBuilder addSignatory(const interface::types::AddressType &account_id,
                                const crypto::PublicKey &public_key);

      /**
       * Removes signatory
       * @param account_id - id of signatory's account to remove
       * @param public_key - public key of signatory
       * @return builder with removed signatory command appended
       */
      ModelTransactionBuilder removeSignatory(
          const interface::types::AddressType &account_id,
          const crypto::PublicKey &public_key);

      /**
       * Creates new account
       * @param account_name - name of account to create
       * @param domain_id - id of domain where account will be created
       * @param main_pubkey - main public key of account
       * @return builder with new account command appended
       */
      ModelTransactionBuilder createAccount(const std::string &account_name,
                                 const interface::types::AddressType &domain_id,
                                 const crypto::PublicKey &main_pubkey);

      /**
       * Creates new domain
       * @param domain_id - domain name to create
       * @param default_role - default role name
       * @return builder with new domain command appended
       */
      ModelTransactionBuilder createDomain(
          const interface::types::AddressType &domain_id,
          const interface::types::RoleIdType &default_role);

      /**
       * Sets account quorum
       * @param account_id - id of account to set quorum
       * @param quorum - quorum amount
       * @return builder with set account quorum command appended
       */
      ModelTransactionBuilder setAccountQuorum(
          const interface::types::AddressType &account_id, uint32_t quorum);

      /**
       * Transfers asset from one account to another
       * @param src_account_id - source account id
       * @param dest_account_id - destination account id
       * @param asset_id - asset id
       * @param description - description message which user can set
       * @param amount - amount of asset to transfer
       * @return buidler with transfer asset command appended
       */
      ModelTransactionBuilder transferAsset(
          const interface::types::AccountIdType &src_account_id,
          const interface::types::AccountIdType &dest_account_id,
          const interface::types::AssetIdType &asset_id,
          const std::string &description,
          const std::string &amount);

      /**
       * Builds result with all appended fields
       * @return wrapper on unsigned transaction
       */
      proto::UnsignedWrapper<proto::Transaction> build();

     private:
      proto::TemplateTransactionBuilder<
          (1 << shared_model::proto::TemplateTransactionBuilder<>::total) - 1>
          builder_;
    };
  }  // namespace bindings
}  // namespace shared_model

#endif  // IROHA_SIMPLE_BUILDER_HPP
