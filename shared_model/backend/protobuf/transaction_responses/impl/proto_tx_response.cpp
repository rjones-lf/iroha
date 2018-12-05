/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "backend/protobuf/transaction_responses/proto_tx_response.hpp"

#include "common/visitor.hpp"
#include "utils/variant_deserializer.hpp"

using Variant =
    shared_model::proto::TransactionResponse::ProtoResponseVariantType;
template Variant::~variant();
template Variant::variant(Variant &&);
template void Variant::destroy_content();
template int Variant::which() const;
template void Variant::indicate_which(int);
template bool Variant::using_backup() const;

namespace shared_model {
  namespace proto {

    template <typename TxResponse>
    TransactionResponse::TransactionResponse(TxResponse &&ref)
        : CopyableProto(std::forward<TxResponse>(ref)),
          variant_([this] {
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
                    std::forward<decltype(ar)>(ar),
                    which > last ? last : which);
          }()),
          ivariant_(variant_),
          hash_(crypto::Hash::fromHexString(proto_->tx_hash())) {}

    template TransactionResponse::TransactionResponse(
        TransactionResponse::TransportType &);
    template TransactionResponse::TransactionResponse(
        const TransactionResponse::TransportType &);
    template TransactionResponse::TransactionResponse(
        TransactionResponse::TransportType &&);

    TransactionResponse::TransactionResponse(const TransactionResponse &r)
        : TransactionResponse(r.proto_) {}

    TransactionResponse::TransactionResponse(TransactionResponse &&r) noexcept
        : TransactionResponse(std::move(r.proto_)) {}

    const interface::types::HashType &TransactionResponse::transactionHash()
        const {
      return hash_;
    }

    const TransactionResponse::ResponseVariantType &TransactionResponse::get()
        const {
      return ivariant_;
    }

    const TransactionResponse::StatelessErrorOrFailedCommandNameType &
    TransactionResponse::statelessErrorOrCommandName() const {
      return proto_->err_or_cmd_name();
    }

    TransactionResponse::FailedCommandIndexType
    TransactionResponse::failedCommandIndex() const {
      return proto_->failed_cmd_index();
    }

    TransactionResponse::ErrorCodeType TransactionResponse::errorCode() const {
      return proto_->error_code();
    }

    int TransactionResponse::priority() const noexcept {
      return iroha::visit_in_place(
          variant_,
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
  };  // namespace proto
}  // namespace shared_model
