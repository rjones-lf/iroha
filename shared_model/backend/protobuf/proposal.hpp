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
#include "common_objects/trivial_proto.hpp"
#include "model/proposal.hpp"
#include <boost/range/numeric.hpp>

#include "block.pb.h"
#include "ordering.pb.h"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace proto {
    class Proposal FINAL : public CopyableProto<interface::Proposal,
                                                iroha::protocol::Proposal,
                                                Proposal> {
     public:
      explicit Proposal(iroha::protocol::Proposal &&proposal)
          : CopyableProto(std::forward<Proposal>(proposal)),
            proposal_(detail::makeReferenceGenerator(
                proto_, &iroha::protocol::Proposal)),
            transactions_([this] { return proposal_->transactions(); }) {}

      Proposal(const Proposal &o) : Proposal(o.proto_) {}

      Proposal(Proposal &&o) noexcept : Proposal(std::move(o.proto_)) {}

      const std::vector<shared_model::Transaction> &transactions() const override {
        return *transactions_;
      }

      uint64_t height() const override {
        return proposal_->height();
      }

     private:
      // lazy
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<const iroha::protocol::Proposal &> proposal_;
      const Lazy<std::vector<Transaction>> proposal_;
    };
  }  // namespace proto
}  // namespace shared_model

#endif  // IROHA_PROPOSAL_HPP_HPP
