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

#ifndef IROHA_CONTAINER_VALIDATOR_HPP
#define IROHA_CONTAINER_VALIDATOR_HPP

#include <boost/format.hpp>
#include "datetime/time.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/iroha_internal/block.hpp"
#include "validators/answer.hpp"

// TODO 22/01/2018 x3medima17: write stateless validator IR-837

namespace shared_model {
  namespace validation {

    /**
     * Class that validates blocks and proposal common fields
     */
    template <typename Iface,
              typename FieldValidator,
              typename TransactionValidator,
              typename TransactionsCollectionValidator>
    class ContainerValidator {
     protected:
      void validateTransaction(
          ReasonsGroupType &reason,
          const interface::Transaction &transaction) const {
        auto answer = transaction_validator_.validate(transaction);
        if (answer.hasErrors()) {
          auto message = (boost::format("Tx: %s") % answer.reason()).str();
          reason.second.push_back(message);
        }
      }
      void validateTransactions(
          ReasonsGroupType &reason,
          const interface::types::TransactionsCollectionType &transactions)
          const {
        auto answer = transactions_collection_validator_.validate(transactions);
        if (answer.hasErrors()) {
          reason.second.push_back(answer.reason());
        }
      }

     public:
      explicit ContainerValidator(
          const FieldValidator &field_validator = FieldValidator(),
          const TransactionsCollectionValidator
              &transactions_collection_validator =
                  TransactionsCollectionValidator(),
          const TransactionValidator &transaction_validator =
              TransactionValidator())
          : transactions_collection_validator_(
                transactions_collection_validator),
            transaction_validator_(transaction_validator),
            field_validator_(field_validator) {}

      Answer validate(const Iface &cont, std::string reason_name) const {
        Answer answer;
        ReasonsGroupType reason;
        reason.first = reason_name;
        field_validator_.validateCreatedTime(reason, cont.createdTime());
        field_validator_.validateHeight(reason, cont.height());
        validateTransactions(reason, cont.transactions());
        if (not reason.second.empty()) {
          answer.addReason(std::move(reason));
        }
        return answer;
      }

     private:
      TransactionsCollectionValidator transactions_collection_validator_;
      TransactionValidator transaction_validator_;

     protected:
      FieldValidator field_validator_;
    };

  }  // namespace validation
}  // namespace shared_model

#endif  // IROHA_CONTAINER_VALIDATOR_HPP
