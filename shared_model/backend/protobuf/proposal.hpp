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

#ifndef IROHA_SHARED_MODEL_PROTO_PROPOSAL_HPP
#define IROHA_SHARED_MODEL_PROTO_PROPOSAL_HPP

#include "backend/protobuf/transaction.hpp"
#include "interfaces/iroha_internal/proposal.hpp"

#include "common_objects/trivial_proto.hpp"

#include "interfaces/common_objects/types.hpp"
#include "proposal.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {

  namespace proto {
    class Proposal final : public CopyableProto<interface::Proposal,
                                                iroha::protocol::Proposal,
                                                Proposal> {
     public:
      template <class ProposalType>
      explicit Proposal(ProposalType &&proposal)
          : CopyableProto(std::forward<ProposalType>(proposal)) {}

      Proposal(const Proposal &o) : Proposal(o.proto_) {}

      Proposal(Proposal &&o) noexcept : Proposal(std::move(o.proto_)) {}

      interface::types::TransactionsCollectionType transactions()
          const override {
        return *transactions_;
      }

      interface::types::TimestampType createdTime() const override {
        return proto_->created_time();
      }

      interface::types::HeightType height() const override {
        return proto_->height();
      }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<std::vector<proto::Transaction>> transactions_{[this] {
        return std::vector<proto::Transaction>(proto_->transactions().begin(),
                                               proto_->transactions().end());
      }};
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROPOSAL_HPP
