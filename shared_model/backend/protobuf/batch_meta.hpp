/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_BATCH_META_HPP
#define IROHA_PROTO_BATCH_META_HPP

#include <boost/range/numeric.hpp>
#include <numeric>

#include "backend/protobuf/common_objects/trivial_proto.hpp"
#include "backend/protobuf/util.hpp"
#include "block.pb.h"
#include "interfaces/common_objects/amount.hpp"
#include "interfaces/common_objects/types.hpp"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class BatchMeta final
        : public CopyableProto<interface::BatchMeta,
                               iroha::protocol::Transaction::Payload::BatchMeta,
                               BatchMeta> {
     public:
      template <typename BatchMetaType>
      explicit BatchMeta(BatchMetaType &&batch_meta)
          : CopyableProto(std::forward<BatchMetaType>(batch_meta)),
            type_{[this] {
              unsigned which = proto_->GetDescriptor()
                                   ->FindFieldByName("type")
                                   ->enum_type()
                                   ->FindValueByNumber(proto_->type())
                                   ->index();
              return static_cast<interface::types::BatchType>(which);
            }},
            transaction_hashes_{[this] {
              return boost::accumulate(
                  proto_->tx_hashes(),
                  TransactionHashesType{},
                  [](auto &&acc, const auto &hash) {
                    acc.emplace_back(hash);
                    return std::forward<decltype(acc)>(acc);
                  });
            }} {}

      BatchMeta(const BatchMeta &o) : BatchMeta(o.proto_) {}

      BatchMeta(BatchMeta &&o) noexcept : BatchMeta(std::move(o.proto_)) {}

      interface::types::BatchType type() const override {
        return *type_;
      };
      const TransactionHashesType &transactionHashes() const override {
        return *transaction_hashes_;
      };

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      Lazy<interface::types::BatchType> type_;

      const Lazy<TransactionHashesType> transaction_hashes_;
    };  // namespace proto
  }     // namespace proto
}  // namespace shared_model
#endif  // IROHA_PROTO_AMOUNT_HPP
