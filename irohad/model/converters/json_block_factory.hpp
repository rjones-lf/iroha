/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
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
        explicit JsonBlockFactory(
            logger::Logger log = logger::log("JsonBlockFactory"));
        rapidjson::Document serialize(const Block &block);

        boost::optional<Block> deserialize(const rapidjson::Document &document);

       private:
        JsonTransactionFactory factory_;
        logger::Logger log_;
      };

    }  // namespace converters
  }    // namespace model
}  // namespace iroha

#endif  // IROHA_JSON_BLOCK_FACTORY_HPP
