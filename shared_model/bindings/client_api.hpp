/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/transaction.hpp"
#include "interfaces/common_objects/types.hpp"
#include "builders/protobuf/unsigned_proto.hpp
#include "cryptography/keypair.hpp"

namespace shared_model {
  namespace bindings {
    using Blob = std::vector<uint8_t>;

    /**
     * Validate protobuf transaction
     * @param blob to validate
     * @return string with errors, empty if none
     */
    void validateTransaction(const Blob &);

    /**
     * Validate protobuf query
     * @param blob to validate
     * @return string with errors, empty if none
     */
    void validateQuery(const Blob &);

    /**
     * Signs protobuf transaction
     * @param blob to sign
     * @param key is keypair for signing
     * @return signed blob
     */
    Blob signTransaction(const Blob &, const crypto::Keypair &);

    /**
     * Signs protobuf query
     * @param blob to sign
     * @param key is keypair for signing
     * @return signed blob
     */
    Blob signQuery(const Blob &, const crypto::Keypair &);

    /**
     * Get the hash of given protobuf transaction
     * @param blob to calculate hash from
     * @return hash of the blob
     */
    Blob hashTransaction(const Blob &);

    /**
     * Get the hash of given protobuf query
     * @param blob to calculate hash from
     * @return hash of the blob
     */
    Blob hashQuery(const Blob &);

    /**
     * Get reduced hash of unsigned transaction
     * @param utx to get hash from
     * @return reduced hash
     */
    interface::types::HashType utxReducedHash(
        const shared_model::proto::UnsignedWrapper<
            shared_model::proto::Transaction> &);

  }  // namespace bindings
}  // namespace shared_model
