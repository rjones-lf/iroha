/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_REJECTED_BEFORE_TX_RESPONSE_HPP
#define IROHA_REJECTED_BEFORE_TX_RESPONSE_HPP

#include "interfaces/transaction_responses/abstract_tx_response.hpp"

namespace shared_model {
  namespace interface {
    /**
     * This transaction has already been submitted, but failed stateful
     * validation. It can not be applied for execution any more (see IR-1769).
     */
    class RejectedBeforeTxResponse
        : public AbstractTxResponse<RejectedBeforeTxResponse> {
     private:
      std::string className() const override {
        return "RejectedBeforeTxResponse";
      }
    };

  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_REJECTED_BEFORE_TX_RESPONSE_HPP
