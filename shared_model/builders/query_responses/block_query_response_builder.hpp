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

#ifndef IROHA_BLOCK_QUERY_RESPONSE_BUILDER_HPP
#define IROHA_BLOCK_QUERY_RESPONSE_BUILDER_HPP

namespace shared_model {
  namespace builder {

    /**
     * Builder to construct transaction status object
     * @tparam BuilderImpl
     */
    template <typename BuilderImpl>
    class BlockQueryResponseBuilder {
     public:
      std::shared_ptr<shared_model::interface::BlockQueryResponse> build() {
        return clone(builder_.build());
      }

      BlockQueryResponseBuilder blockResponse(
          shared_model::interface::Block &block) {
        BlockQueryResponseBuilder copy(*this);
        copy.builder_ = this->builder_.blockResponse(block);
        return copy;
      }

      BlockQueryResponseBuilder errorResponse(std::string &message) {
        BlockQueryResponseBuilder copy(*this);
        copy.builder_ = this->builder_.errorResponse(message);
        return copy;
      }

     private:
      BuilderImpl builder_;
    };

  }  // namespace builder
}  // namespace shared_model

#endif  // IROHA_BLOCK_QUERY_RESPONSE_BUILDER_HPP
