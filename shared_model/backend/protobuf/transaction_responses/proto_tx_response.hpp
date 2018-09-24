/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_TX_RESPONSE_HPP
#define IROHA_PROTO_TX_RESPONSE_HPP

#include <limits>

#include "backend/protobuf/transaction_responses/proto_concrete_tx_response.hpp"
#include "common/visitor.hpp"
#include "utils/lazy_initializer.hpp"
#include "utils/variant_deserializer.hpp"

namespace shared_model {
  namespace proto {
    /**
     * TransactionResponse is a status of transaction in system
     */
    class TransactionResponse final
        : public CopyableProto<interface::TransactionResponse,
                               iroha::protocol::ToriiResponse,
                               TransactionResponse> {
     public:
      /// Type of variant, that handle all concrete tx responses in the system
      using ProtoResponseVariantType =
          boost::variant<StatelessFailedTxResponse,
                         StatelessValidTxResponse,
                         StatefulFailedTxResponse,
                         StatefulValidTxResponse,
                         RejectedTxResponse,
                         CommittedTxResponse,
                         MstExpiredResponse,
                         NotReceivedTxResponse,
                         MstPendingResponse,
                         EnoughSignaturesCollectedResponse>;

      /// Type with list of types in ResponseVariantType
      using ProtoResponseListType = ProtoResponseVariantType::types;

      template <typename TxResponse>
      explicit TransactionResponse(TxResponse &&ref)
          : CopyableProto(std::forward<TxResponse>(ref)) {}

      TransactionResponse(const TransactionResponse &r)
          : TransactionResponse(r.proto_) {}

      TransactionResponse(TransactionResponse &&r) noexcept
          : TransactionResponse(std::move(r.proto_)) {}

      const interface::types::HashType &transactionHash() const override {
        return *hash_;
      }

      /**
       * @return attached interface tx response
       */
      const ResponseVariantType &get() const override {
        return *ivariant_;
      }

      const ErrorMessageType &errorMessage() const override {
        return proto_->error_message();
      }

     private:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      /// lazy variant shortcut
      using LazyVariantType = Lazy<ProtoResponseVariantType>;

      // lazy
      const LazyVariantType variant_{detail::makeLazyInitializer([this] {
        auto &&ar = *proto_;

        unsigned which = ar.GetDescriptor()
                             ->FindFieldByName("tx_status")
                             ->enum_type()
                             ->FindValueByNumber(ar.tx_status())
                             ->index();
        constexpr unsigned last =
            boost::mpl::size<ProtoResponseListType>::type::value - 1;

        return shared_model::detail::variant_impl<ProtoResponseListType>::
            template load<shared_model::proto::TransactionResponse::
                              ProtoResponseVariantType>(
                std::forward<decltype(ar)>(ar), which > last ? last : which);
      })};

      const Lazy<ResponseVariantType> ivariant_{detail::makeLazyInitializer(
          [this] { return ResponseVariantType(*variant_); })};

      // stub hash
      const Lazy<crypto::Hash> hash_{
          [this] { return crypto::Hash(this->proto_->tx_hash()); }};

      static constexpr int max_priority = std::numeric_limits<int>::max();
      int priority() const noexcept override {
        return iroha::visit_in_place(
            *variant_,
            // not received can be changed to any response
            [](const NotReceivedTxResponse &) { return 0; },
            // following types are sequential in pipeline
            [](const StatelessValidTxResponse &) { return 1; },
            [](const MstPendingResponse &) { return 2; },
            [](const EnoughSignaturesCollectedResponse &) { return 3; },
            [](const StatefulValidTxResponse &) { return 4; },
            // following types are local on this peer and can be substituted by
            // final ones, if consensus decides so
            [](const StatelessFailedTxResponse &) { return 5; },
            [](const StatefulFailedTxResponse &) { return 5; },
            [](const MstExpiredResponse &) { return 5; },
            // following types are the final ones
            [](const CommittedTxResponse &) { return max_priority; },
            [](const RejectedTxResponse &) { return max_priority; });
      }
    };
  }  // namespace  proto
}  // namespace shared_model
#endif  // IROHA_PROTO_TX_RESPONSE_HPP
