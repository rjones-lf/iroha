/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TX_RESPONSE_HPP
#define IROHA_TX_RESPONSE_HPP

#include <boost/variant.hpp>

#include "interfaces/base/model_primitive.hpp"
#include "interfaces/transaction.hpp"
#include "interfaces/transaction_responses/committed_tx_response.hpp"
#include "interfaces/transaction_responses/enough_signatures_collected_response.hpp"
#include "interfaces/transaction_responses/mst_expired_response.hpp"
#include "interfaces/transaction_responses/mst_pending_response.hpp"
#include "interfaces/transaction_responses/not_received_tx_response.hpp"
#include "interfaces/transaction_responses/rejected_tx_response.hpp"
#include "interfaces/transaction_responses/stateful_failed_tx_response.hpp"
#include "interfaces/transaction_responses/stateful_valid_tx_response.hpp"
#include "interfaces/transaction_responses/stateless_failed_tx_response.hpp"
#include "interfaces/transaction_responses/stateless_valid_tx_response.hpp"
#include "utils/visitor_apply_for_all.hpp"

namespace shared_model {
  namespace interface {
    /**
     * TransactionResponse is a status of transaction in system
     */
    class TransactionResponse : public ModelPrimitive<TransactionResponse> {
     private:
      /// const reference shortcut type
      template <typename... Value>
      using wrap = boost::variant<const Value &...>;

     protected:
      /**
       * @return priority of this transaction response; transaction response can
       * only be replaced with one with higher priority
       */
      virtual int priority() const noexcept = 0;

     public:
      /// Type of variant, that handle all concrete tx responses in the system
      using ResponseVariantType = wrap<StatelessFailedTxResponse,
                                       StatelessValidTxResponse,
                                       StatefulFailedTxResponse,
                                       StatefulValidTxResponse,
                                       RejectTxResponse,
                                       CommittedTxResponse,
                                       MstExpiredResponse,
                                       NotReceivedTxResponse,
                                       MstPendingResponse,
                                       EnoughSignaturesCollectedResponse>;

      /// Type with list of types in ResponseVariantType
      using ResponseListType = ResponseVariantType::types;

      /// Type of transaction hash
      using TransactionHashType = interface::types::HashType;

      /**
       * @return hash of corresponding transaction
       */
      virtual const TransactionHashType &transactionHash() const = 0;

      /**
       * @return attached concrete tx response
       */
      virtual const ResponseVariantType &get() const = 0;

      /// Message type
      using ErrorMessageType = std::string;

      /**
       * @return error message if present, otherwise - an empty string
       */
      virtual const ErrorMessageType &errorMessage() const = 0;

      /**
       * Enumeration for holding result of priorities comparison
       */
      enum class PrioritiesComparisonResult { kLess, kEqual, kGreater };
      /**
       * Compare priorities of two transaction responses
       * @param other response
       * @return enumeration result of that comparison
       */
      PrioritiesComparisonResult comparePriorities(const ModelType &other) const
          noexcept {
        if (this->priority() < other.priority()) {
          return PrioritiesComparisonResult::kLess;
        } else if (this->priority() == other.priority()) {
          return PrioritiesComparisonResult::kEqual;
        }
        return PrioritiesComparisonResult::kGreater;
      };

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("TransactionResponse")
            .append("transactionHash", transactionHash().hex())
            .append(boost::apply_visitor(detail::ToStringVisitor(), get()))
            .append("errorMessage", errorMessage())
            .finalize();
      }

      bool operator==(const ModelType &rhs) const override {
        return transactionHash() == rhs.transactionHash()
            and errorMessage() == rhs.errorMessage() and get() == rhs.get();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_TX_RESPONSE_HPP
