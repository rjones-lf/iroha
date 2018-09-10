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

#ifndef IROHA_SHARED_MODEL_PROPOSAL_HPP
#define IROHA_SHARED_MODEL_PROPOSAL_HPP

#include "cryptography/default_hash_provider.hpp"
#include "interfaces/base/model_primitive.hpp"
#include "interfaces/common_objects/types.hpp"
#include "interfaces/transaction.hpp"
#include "utils/lazy_initializer.hpp"

namespace shared_model {
  namespace interface {

    class Proposal : public ModelPrimitive<Proposal> {
     public:
      /**
       * @return transactions
       */
      virtual types::TransactionsCollectionType transactions() const = 0;

      /**
       * @return the height
       */
      virtual types::HeightType height() const = 0;

      /**
       * @return created time
       */
      virtual types::TimestampType createdTime() const = 0;

      bool operator==(const Proposal &rhs) const override {
        return transactions() == rhs.transactions() and height() == rhs.height()
            and createdTime() == rhs.createdTime();
      }

      virtual const types::BlobType &blob() const = 0;

      const types::HashType &hash() const {
        return *hash_;
      }

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("Proposal")
            .append("height", std::to_string(height()))
            .append("transactions")
            .appendAll(transactions(),
                       [](auto &transaction) { return transaction.toString(); })
            .finalize();
      }

     protected:
      template <typename T>
      using Lazy = detail::LazyInitializer<T>;

      const Lazy<types::HashType> hash_{
          [this] { return crypto::DefaultHashProvider::makeHash(blob()); }};
    };

  }  // namespace interface
}  // namespace shared_model

#endif  // IROHA_SHARED_MODEL_PROPOSAL_HPP
