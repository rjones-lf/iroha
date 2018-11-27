/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_TX_RESPONSE_HPP
#define IROHA_PROTO_TX_RESPONSE_HPP

#include <limits>

#include "backend/protobuf/transaction_responses/proto_concrete_tx_response.hpp"

namespace shared_model {
  namespace proto {
    /**
     * TransactionResponse is a status of transaction in system
     */
    class TransactionResponse final : public interface::TransactionResponse {
     public:
      using TransportType = iroha::protocol::ToriiResponse;

      TransactionResponse(const TransactionResponse &r);
      TransactionResponse(TransactionResponse &&r) noexcept;

      explicit TransactionResponse(const TransportType &ref);
      explicit TransactionResponse(TransportType &&ref);

      ~TransactionResponse() override;

      const interface::types::HashType &transactionHash() const override;

      /**
       * @return attached interface tx response
       */
      const ResponseVariantType &get() const override;

      const StatelessErrorOrFailedCommandNameType &statelessErrorOrCommandName()
          const override;

      FailedCommandIndexType failedCommandIndex() const override;

      ErrorCodeType errorCode() const override;

      const TransportType &getTransport() const;

     protected:
      TransactionResponse *clone() const override;

     private:
      struct Impl;
      std::unique_ptr<Impl> impl_;

      static constexpr int max_priority = std::numeric_limits<int>::max();
      int priority() const noexcept override;
    };
  }  // namespace  proto
}  // namespace shared_model

#endif  // IROHA_PROTO_TX_RESPONSE_HPP
