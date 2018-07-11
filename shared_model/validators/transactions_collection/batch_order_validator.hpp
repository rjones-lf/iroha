/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_BATCH_ORDER_VALIDATOR_HPP
#define IROHA_BATCH_ORDER_VALIDATOR_HPP

#include "order_validator.hpp"

namespace shared_model {
  namespace validation {
    class BatchOrderValidator : public OrderValidator {
     private:
      /**
       * check order of transaction, recieves 2 consequtive transactions in
       * sequence and check weater they can be a part of correct sequence
       * @param t1, t2 transactions to check the order of boost::none to specify
       * beginning or end of sequence
       * @return empty string if order is correct or error message
       */
      std::string canFollow(
          boost::optional<const interface::Transaction &> tr1,
          boost::optional<const interface::Transaction &> tr2) const;

     public:
      virtual Answer validate(
          const interface::types::TransactionsForwardCollectionType
              &transactions) const override;
    };
  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_ORDER_VALIDATOR_HPP
