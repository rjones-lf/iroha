/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_TX_PAGINATION_META_VALIDATOR_HPP
#define IROHA_TX_PAGINATION_META_VALIDATOR_HPP

#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

    /**
     * Class that validates proposal
     */
    template <typename FieldValidator>
    class TxPaginationMetaValidator {
     public:
      /**
       * Applies validation on proposal
       * @param proposal
       * @return Answer containing found error if any
       */
      Answer validate(const interface::TxPaginationMeta &tx_pagination_meta) const {
        ReasonsGroupType reason;
        reason.first = "Transaction pagination metadata";
        field_validator_.validateTxPaginationMeta(reason, tx_pagination_meta);

        Answer answer;
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }
        return answer;
      }

     protected:
      FieldValidator field_validator_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_TX_PAGINATION_META_VALIDATOR_HPP
