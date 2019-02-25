/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_MODEL_PROTO_HPP
#define IROHA_SHARED_MODEL_MODEL_PROTO_HPP

#include "builders/protobuf/transaction.hpp"
#include "builders/protobuf/unsigned_proto.hpp"
#include "cryptography/blob.hpp"

namespace shared_model {
  namespace bindings {
    /**
     * Class for proto operations for SWIG
     * @tparam UnsignedWrapper - type of unsigned model proto object
     */
    template <typename UnsignedWrapper>
    class ModelProto {
     public:
      ModelProto(UnsignedWrapper &us) : us_(us) {}
      /**
       * Signs and adds a signature for a proto object
       * @param keypair - keypair to sign
       * @return ModelProto with signed object
       */
      ModelProto<UnsignedWrapper> signAndAddSignature(
          const crypto::Keypair &keypair) {
        UnsignedWrapper wrapper = us_.signAndAddSignature(keypair);
        return ModelProto<UnsignedWrapper>(wrapper);
      }

      /**
       * Finishes object building
       * @return built object
       */
      crypto::Blob finish() {
        return us_.finish().blob();
      }

     private:
      UnsignedWrapper us_;
    };
  }  // namespace bindings
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_MODEL_PROTO_HPP
