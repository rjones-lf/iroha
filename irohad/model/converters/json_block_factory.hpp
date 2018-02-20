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

#ifndef IROHA_JSON_BLOCK_FACTORY_HPP
#define IROHA_JSON_BLOCK_FACTORY_HPP

#include "logger/logger.hpp"
#include "model/block.hpp"
#include "model/converters/json_transaction_factory.hpp"

namespace iroha {
  namespace model {
    namespace converters {

      class JsonBlockFactory {
       public:
        JsonBlockFactory();
        rapidjson::Document serialize(const Block &block);

        boost::optional<Block> deserialize(
            const rapidjson::Document &document);

       private:
        JsonTransactionFactory factory_;
        logger::Logger log_;
      };

    }  // namespace converters
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_JSON_BLOCK_FACTORY_HPP
