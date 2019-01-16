/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
    class DEPRECATED SignatureBuilder {
     public:
      shared_model::proto::Signature build() {
        return shared_model::proto::Signature(
            iroha::protocol::Signature(signature_));
      }

      SignatureBuilder publicKey(
          const shared_model::interface::types::PubkeyType &key) {
        SignatureBuilder copy(*this);
        copy.signature_.set_public_key(key.hex());
        return copy;
      }

      SignatureBuilder signedData(
          const interface::Signature::SignedType &signed_data) {
        SignatureBuilder copy(*this);
        copy.signature_.set_signature(signed_data.hex());
        return copy;
      }

     private:
      iroha::protocol::Signature signature_;
    };
  }  // namespace proto
}  // namespace shared_model
#endif  // IROHA_PROTO_SIGNATURE_BUILDER_HPP
