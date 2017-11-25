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

#ifndef IROHA_PROTO_TX_RESPONSE_HPP
#define IROHA_PROTO_TX_RESPONSE_HPP

#include "endpoint.pb.h"
#include "interfaces/transaction_responses/committed_tx_response.hpp"
#include "interfaces/transaction_responses/stateful_failed_tx_response.hpp"
#include "interfaces/transaction_responses/stateful_valid_tx_response.hpp"
#include "interfaces/transaction_responses/stateless_failed_tx_response.hpp"
#include "interfaces/transaction_responses/stateless_valid_tx_response.hpp"
#include "interfaces/transaction_responses/tx_response.hpp"
#include "interfaces/transaction_responses/unknown_tx_response.hpp"
#include "utils/lazy_initializer.hpp"
#include "utils/reference_holder.hpp"
#include "utils/variant_deserializer.hpp"

template <typename... T>
auto load(const iroha::protocol::ToriiResponse &ar) {
  unsigned which = ar.tx_status();
  constexpr unsigned last = boost::mpl::size<T...>::type::value - 1;

  return shared_model::detail::variant_impl<T...>::template load<
      shared_model::interface::TransactionResponse::ResponseVariantType>(
      ar, which > last ? last : which);
  ;
}

namespace shared_model {
  namespace proto {
    template <typename Iface>
    class RespType final : public Iface {
     private:
      using RefToriiResp =
          detail::ReferenceHolder<iroha::protocol::ToriiResponse>;

     public:
      explicit RespType(const iroha::protocol::ToriiResponse &response)
          : RespType(RefToriiResp(response)) {}

      explicit RespType(iroha::protocol::ToriiResponse &&response)
          : RespType(RefToriiResp(std::move(response))) {}

      typename Iface::ModelType *copy() const override {
        return new RespType(*response_);
      }

     private:
      explicit RespType(RefToriiResp &&ref) : response_(std::move(ref)) {}
      // proto
      RefToriiResp response_;
    };

    using StatelessFailedTxResponse =
        RespType<interface::StatelessFailedTxResponse>;
    using StatelessValidTxResponse =
        RespType<interface::StatelessValidTxResponse>;
    using StatefulFailedTxResponse =
        RespType<interface::StatefulFailedTxResponse>;
    using StatefulValidTxResponse =
        RespType<interface::StatefulValidTxResponse>;
    using CommittedTxResponse = RespType<interface::CommittedTxResponse>;
    using UnknownTxResponse = RespType<interface::UnknownTxResponse>;

    /**
     * TransactionResponse is a status of transaction in system
     */
    class TransactionResponse final : public interface::TransactionResponse {
     private:
      /// PolymorphicWrapper shortcut type
      template <typename... Value>
      using wrap = boost::variant<detail::PolymorphicWrapper<Value>...>;

      /// lazy variant shortcut
      using LazyVariantType = detail::LazyInitializer<ResponseVariantType>;

      using RefTxResponse =
          detail::ReferenceHolder<iroha::protocol::ToriiResponse>;

     public:
      /// Type of variant, that handle all concrete tx responses in the system
      using ProtoResponseVariantType = wrap<StatelessFailedTxResponse,
                                            StatelessValidTxResponse,
                                            StatefulFailedTxResponse,
                                            StatefulValidTxResponse,
                                            CommittedTxResponse,
                                            UnknownTxResponse>;

      /// Type with list of types in ResponseVariantType
      using ProtoResponseListType = ProtoResponseVariantType::types;

      explicit TransactionResponse(
          const iroha::protocol::ToriiResponse &command)
          : TransactionResponse(RefTxResponse(command)) {}

      explicit TransactionResponse(iroha::protocol::ToriiResponse &&command)
          : TransactionResponse(RefTxResponse(std::move(command))) {}

      /**
       * @return hash of corresponding transaction
       */
      const interface::Transaction::HashType &transactionHash() const override {
        return hash_;
      };

      /**
       * @return attached concrete tx response
       */
      const ResponseVariantType &get() const override {
        return *variant_;
      }

      ModelType *copy() const override {
        return new TransactionResponse(
            iroha::protocol::ToriiResponse(*response_));
      }

     private:
      explicit TransactionResponse(RefTxResponse &&ref)
          : response_(std::move(ref)),
            variant_(detail::makeLazyInitializer([this] {
              return ResponseVariantType(
                  load<ProtoResponseListType>(*response_));
            })),
            // fixme @l4l: proto should be changed and this one replaced as well
            //             or some other solution needed
            hash_("") {}

      // ------------------------------| fields |-------------------------------

      // proto
      RefTxResponse response_;

      // lazy
      LazyVariantType variant_;

      // stub hash
      crypto::Hash hash_;
    };
  }  // namespace  proto
}  // namespace shared_model
#endif  // IROHA_PROTO_TX_RESPONSE_HPP
