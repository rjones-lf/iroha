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

#ifndef IROHA_PBFACTORY_HPP
#define IROHA_PBFACTORY_HPP

#include "block.pb.h"
#include "model/block.hpp"

namespace iroha {
  namespace model {
    namespace converters {

      /**
       * Converting business objects to protobuf and vice versa
       */
      class PbBlockFactory {
       public:
        PbBlockFactory() {}

        /**
         * Convert block to proto block
         * @param block - reference to block
         * @return proto block
         */
        protocol::Block serialize(const model::Block &block) const;

        /**
         * Convert proto block to model block
         * @param pb_block - reference to proto block
         * @return model block
         */
        model::Block deserialize(const protocol::Block &pb_block) const;
      };
    }  // namespace converters
  }    // namespace model
}  // namespace iroha
#endif  // IROHA_PBFACTORY_HPP
