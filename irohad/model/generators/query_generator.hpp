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
#include "model/queries/get_account.hpp"
#include "model/queries/get_account_assets.hpp"
#include "model/queries/get_signatories.hpp"
#include "model/queries/get_transactions.hpp"

#include "model/model_hash_provider_impl.hpp"

#ifndef IROHA_QUERY_GENERATOR_HPP
#define IROHA_QUERY_GENERATOR_HPP
namespace iroha {
  namespace model {
    namespace generators {
      class QueryGenerator {
       public:
        /**
         *
         * @param timestamp
         * @param creator
         * @param query_counter
         * @param account_id
         * @return
         */
        GetAccount generateGetAccount(ts64_t timestamp, std::string creator,
                                      uint64_t query_counter,
                                      std::string account_id);
        /**
         *
         * @param timestamp
         * @param creator
         * @param query_counter
         * @param account_id
         * @param asset_id
         * @return
         */
        GetAccountAssets generateGetAccountAssets(ts64_t timestamp,
                                                  std::string creator,
                                                  uint64_t query_counter,
                                                  std::string account_id,
                                                  std::string asset_id);
        /**
         *
         * @param timestamp
         * @param creator
         * @param query_counter
         * @param account_id
         * @return
         */
        GetSignatories generateGetSignatories(ts64_t timestamp,
                                              std::string creator,
                                              uint64_t query_counter,
                                              std::string account_id);
        /**
         *
         * @param timestamp
         * @param creator
         * @param query_counter
         * @param account_id
         * @return
         */
        GetAccountTransactions generateGetAccountTransactions(
            ts64_t timestamp, std::string creator, uint64_t query_counter,
            std::string account_id);

       private:
        HashProviderImpl hash_provider_;
      };
    }  // namespace generators
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_QUERY_GENERATOR_HPP
