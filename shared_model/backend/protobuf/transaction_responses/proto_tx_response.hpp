/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_TX_RESPONSE_HPP
#define IROHA_PROTO_TX_RESPONSE_HPP

#include <limits>

#include "backend/protobuf/transaction_responses/proto_concrete_tx_response.hpp"
#include "cryptography/hash.hpp"

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
      explicit TransactionResponse(TxResponse &&ref);

      TransactionResponse(const TransactionResponse &r);

      TransactionResponse(TransactionResponse &&r) noexcept;

      const interface::types::HashType &transactionHash() const override;

      /**
       * @return attached interface tx response
       */
      const ResponseVariantType &get() const override;

      const ErrorMessageType &errorMessage() const override;

     private:
      const ProtoResponseVariantType variant_;

      const ResponseVariantType ivariant_;

      // stub hash
      const crypto::Hash hash_;

      static constexpr int max_priority = std::numeric_limits<int>::max();
      int priority() const noexcept override;
    };
  }  // namespace  proto
}  // namespace shared_model

namespace boost {
    extern template class variant<shared_model::proto::StatelessFailedTxResponse,
            shared_model::proto::StatelessValidTxResponse,
            shared_model::proto::StatefulFailedTxResponse,
            shared_model::proto::StatefulValidTxResponse,
            shared_model::proto::RejectedTxResponse,
            shared_model::proto::CommittedTxResponse,
            shared_model::proto::MstExpiredResponse,
            shared_model::proto::NotReceivedTxResponse,
            shared_model::proto::MstPendingResponse,
            shared_model::proto::EnoughSignaturesCollectedResponse>;
}

#endif  // IROHA_PROTO_TX_RESPONSE_HPP
