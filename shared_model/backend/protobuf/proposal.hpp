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

#ifndef IROHA_SHARED_MODEL_PROTO_PROPOSAL_HPP
#define IROHA_SHARED_MODEL_PROTO_PROPOSAL_HPP

#include "interfaces/proposal.hpp"
#include "interfaces/transaction.hpp"

#include <boost/range/numeric.hpp>
#include "common_objects/trivial_proto.hpp"
#include "model/proposal.hpp"

#include "block.pb.h"
#include "ordering.pb.h"
#include "utils/lazy_initializer.hpp"

#include "transaction.hpp"
namespace shared_model {
  namespace proto {
    class Proposal FINAL
        : public CopyableProto<interface::Proposal,
                               iroha::ordering::proto::Proposal,
                               Proposal> {
     public:
      template <class ProposalType>
      explicit Proposal(ProposalType &&proposal)
          : CopyableProto(
                std::forward<ProposalType>(proposal)),
            transactions_([this] {
              std::vector<wTransaction> txs;
              for(const auto& tx: proto_->transactions()){
                auto tmp = detail::make_polymorphic<proto::Transaction>(tx);
                txs.emplace_back(tmp);
              }
              return txs;
            }) {}

      Proposal(const Proposal &o) : Proposal(o.proto_) {}

      Proposal(Proposal &&o) noexcept : Proposal(std::move(o.proto_)) {}

      const std::vector<wTransaction> &transactions() const override {
        return *transactions_;
      }

      uint64_t height() const override {
        return proto_->height();
      }

     private:
      using wTransaction = detail::PolymorphicWrapper<interface::Transaction>;

      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<std::vector<wTransaction>> transactions_;

    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROPOSAL_HPP_HPP
