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
