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

#ifndef IROHA_SHARED_MODEL_PROTO_BLOCKS_QUERY_HPP
#define IROHA_SHARED_MODEL_PROTO_BLOCKS_QUERY_HPP

#include "backend/protobuf/common_objects/signature.hpp"
#include "interfaces/queries/blocks_query.hpp"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class BlocksQuery FINAL : public CopyableProto<interface::BlocksQuery,
                                                   iroha::protocol::BlocksQuery,
                                                   BlocksQuery> {
     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;
     public:
      template <typename BlocksQueryType>
      explicit BlocksQuery(BlocksQueryType &&query)
          : CopyableProto(std::forward<BlocksQueryType>(query)) {}

      BlocksQuery(const BlocksQuery &o) : BlocksQuery(o.proto_) {}

      BlocksQuery(BlocksQuery &&o) noexcept : BlocksQuery(std::move(o.proto_)) {}

      const interface::types::AccountIdType &creatorAccountId() const override {
        return proto_->meta().creator_account_id();
      }

      interface::types::CounterType queryCounter() const override {
        return proto_->meta().query_counter();
      }

      const interface::types::BlobType &blob() const override {
        return *blob_;
      }

      const interface::types::BlobType &payload() const override {
        return *payload_;
      }

      // ------------------------| Signable override  |-------------------------
      const interface::SignatureSetType &signatures() const override {
        return *signatures_;
      }

      bool addSignature(const crypto::Signed &signed_blob,
                        const crypto::PublicKey &public_key) override {
        if (proto_->has_signature()) {
          return false;
        }

        auto sig = proto_->mutable_signature();
        sig->set_signature(crypto::toBinaryString(signed_blob));
        sig->set_pubkey(crypto::toBinaryString(public_key));
        return true;
      }

      bool clearSignatures() override {
        signatures_->clear();
        return (signatures_->size() == 0);
      }

      interface::types::TimestampType createdTime() const override {
        return proto_->meta().created_time();
      }

     private:
      // ------------------------------| fields |-------------------------------
      // lazy
      const Lazy<interface::types::BlobType> blob_{
          [this] { return makeBlob(*proto_); }};

      const Lazy<interface::types::BlobType> payload_{
          [this] { return makeBlob(proto_->meta()); }};

      const Lazy<interface::SignatureSetType> signatures_{[this] {
        interface::SignatureSetType set;
        if (proto_->has_signature()) {
          set.emplace(new Signature(proto_->signature()));
        }
        return set;
      }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROTO_BLOCKS_QUERY_HPP
