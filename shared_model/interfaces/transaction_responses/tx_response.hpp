/**
 * Copyright Soramitsu Co., Ltd. 2017 All Rights Reserved.
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

#ifndef IROHA_TX_RESPONSE_HPP
#define IROHA_TX_RESPONSE_HPP

#include <boost/variant.hpp>
#include "interfaces/base/primitive.hpp"
#include "interfaces/transaction.hpp"
#include "interfaces/transaction_responses/committed_tx_response.hpp"
#include "interfaces/transaction_responses/not_received_tx_response.hpp"
#include "interfaces/transaction_responses/stateful_failed_tx_response.hpp"
#include "interfaces/transaction_responses/stateful_valid_tx_response.hpp"
#include "interfaces/transaction_responses/stateless_failed_tx_response.hpp"
#include "interfaces/transaction_responses/stateless_valid_tx_response.hpp"
#include "utils/polymorphic_wrapper.hpp"
#include "utils/visitor_apply_for_all.hpp"

#ifndef DISABLE_BACKWARD
#include "model/transaction_response.hpp"
#endif

namespace shared_model {
  namespace interface {
    /**
     * TransactionResponse is a status of transaction in system
     */
    class TransactionResponse : public PRIMITIVE(TransactionResponse) {
     private:
      /// PolymorphicWrapper shortcut type
      template <typename... Value>
      using wrap = boost::variant<detail::PolymorphicWrapper<Value>...>;

     public:
      /// Type of variant, that handle all concrete tx responses in the system
      using ResponseVariantType = wrap<StatelessFailedTxResponse,
                                       StatelessValidTxResponse,
                                       StatefulFailedTxResponse,
                                       StatefulValidTxResponse,
                                       CommittedTxResponse,
                                       NotReceivedTxResponse>;

      /// Type with list of types in ResponseVariantType
      using ResponseListType = ResponseVariantType::types;

      /**
       * @return hash of corresponding transaction
       */
      virtual const interface::types::HashType &transactionHash() const = 0;

      /**
       * @return attached concrete tx response
       */
      virtual const ResponseVariantType &get() const = 0;

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return boost::apply_visitor(detail::ToStringVisitor(), get());
      }

#ifndef DISABLE_BACKWARD
      OldModelType *makeOldModel() const override {
        auto response = boost::apply_visitor(
            detail::OldModelCreatorVisitor<OldModelType *>(), get());
        response->tx_hash = crypto::toBinaryString(transactionHash());
        return response;
      }

#endif

      bool operator==(const ModelType &rhs) const override {
        return transactionHash() == rhs.transactionHash()
            and get() == rhs.get();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_TX_RESPONSE_HPP
