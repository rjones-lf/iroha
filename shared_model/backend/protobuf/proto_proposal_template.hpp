/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_PROTO_PROPOSAL_TEMPLATE_HPP
#define IROHA_PROTO_PROPOSAL_TEMPLATE_HPP

#include "interfaces/iroha_internal/proposal.hpp"
#include "interfaces/transaction.hpp"

#include <boost/range/numeric.hpp>
#include "common_objects/trivial_proto.hpp"

#include "block.pb.h"
#include "interfaces/common_objects/types.hpp"
#include "proposal.pb.h"
#include "utils/lazy_initializer.hpp"

#include "transaction.hpp"

namespace shared_model {
  namespace proto {

    /**
     * Template class to create Proposal and Verified Proposal proto shared
     * model types
     * @tparam ParentType either proposal or verified proposal interface object
     */
    template <typename ParentType>
    class ProtoProposalTemplate final
        : public CopyableProto<ParentType,
                               iroha::protocol::Proposal,
                               ProtoProposalTemplate<ParentType>> {
      template <class T>
      using w = detail::PolymorphicWrapper<T>;
      using TransactionContainer = std::vector<w<interface::Transaction>>;

      using CopyableProtoType =
          CopyableProto<ParentType,
                        iroha::protocol::Proposal,
                        ProtoProposalTemplate<ParentType>>;

     public:
      template <class ProposalType>
      explicit ProtoProposalTemplate(ProposalType &&proposal)
          : CopyableProtoType(std::forward<ProposalType>(proposal)),
            transactions_([this] {
              return boost::accumulate(
                  CopyableProtoType::proto_->transactions(),
                  TransactionContainer{},
                  [](auto &&vec, const auto &tx) -> decltype(auto) {
                    vec.emplace_back(new proto::Transaction(tx));
                    return std::forward<decltype(vec)>(vec);
                  });
            }()) {}

      ProtoProposalTemplate(const ProtoProposalTemplate<ParentType> &o)
          : ProtoProposalTemplate(o.proto_) {}

      ProtoProposalTemplate(ProtoProposalTemplate<ParentType> &&o) noexcept
          : ProtoProposalTemplate(std::move(o.proto_)) {}

      const TransactionContainer &transactions() const override {
        return transactions_;
      }

      interface::types::TimestampType createdTime() const override {
        return CopyableProtoType::proto_->created_time();
      }

      interface::types::HeightType height() const override {
        return CopyableProtoType::proto_->height();
      }

     private:
      const TransactionContainer transactions_;
    };

  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROTO_PROPOSAL_TEMPLATE_HPP
