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

#ifndef IROHA_PROTO_SIGNATURE_BUILDER_HPP
#define IROHA_PROTO_SIGNATURE_BUILDER_HPP

#include "backend/protobuf/common_objects/signature.hpp"
#include "interfaces/common_objects/types.hpp"
#include "primitive.pb.h"

namespace shared_model {
  namespace proto {

    /**
     * SignatureBuilder is used to construct Signature proto objects with
     * initialized protobuf implementation
     */
    class SignatureBuilder {
     public:
      shared_model::proto::Signature build() {
        return shared_model::proto::Signature(
            iroha::protocol::Signature(signature_));
      }

      SignatureBuilder publicKey(
          const shared_model::interface::types::PubkeyType &key) {
        SignatureBuilder copy(*this);
        copy.signature_.set_pubkey(shared_model::crypto::toBinaryString(key));
        return copy;
      }

      SignatureBuilder signedData(
          const interface::Signature::SignedType &signed_data) {
        SignatureBuilder copy(*this);
        copy.signature_.set_signature(
            shared_model::crypto::toBinaryString(signed_data));
        return copy;
      }

     private:
      iroha::protocol::Signature signature_;
    };
  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_PROTO_SIGNATURE_BUILDER_HPP
