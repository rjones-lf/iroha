/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_ABSTRACT_PROPOSAL_HPP
#define IROHA_SHARED_MODEL_ABSTRACT_PROPOSAL_HPP

#include <boost/range/numeric.hpp>
#include <vector>
#include "interfaces/base/primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/transaction.hpp"
#include "utils/polymorphic_wrapper.hpp"

#ifndef DISABLE_BACKWARD
#include "model/proposal.hpp"
#endif

namespace shared_model {
  namespace interface {

    class AbstractProposal
#ifndef DISABLE_BACKWARD
        : public Primitive<AbstractProposal, iroha::model::Proposal> {
#else
        : public ModelPrimitive<Model> {
#endif
     public:
      template <class T>
      using w = detail::PolymorphicWrapper<T>;
      using TransactionContainer = std::vector<w<interface::Transaction>>;

      /**
       * @return transactions
       */
      virtual const std::vector<w<Transaction>> &transactions() const = 0;

      /**
       * @return the height
       */
      virtual types::HeightType height() const = 0;

      /**
       * @return created time
       */
      virtual types::TimestampType createdTime() const = 0;

#ifndef DISABLE_BACKWARD
      iroha::model::Proposal *makeOldModel() const override {
        auto txs =
            boost::accumulate(transactions(),
                              std::vector<iroha::model::Transaction>{},
                              [](auto &&vec, const auto &tx) {
                                std::unique_ptr<iroha::model::Transaction> ptr(
                                    tx->makeOldModel());
                                vec.emplace_back(*ptr);
                                return std::forward<decltype(vec)>(vec);
                              });

        auto oldModel = new iroha::model::Proposal(txs);
        oldModel->height = height();
        oldModel->created_time = createdTime();
        return oldModel;
      }
#endif

      bool operator==(const AbstractProposal &rhs) const override {
        return transactions() == rhs.transactions() and height() == rhs.height()
            and createdTime() == rhs.createdTime();
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Proposal")
            .append("height", std::to_string(height()))
            .append("transactions")
            .appendAll(
                transactions(),
                [](auto &transaction) { return transaction->toString(); })
            .finalize();
      }
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_ABSTRACT_PROPOSAL_HPP
