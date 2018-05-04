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

#ifndef IROHA_SHARED_MODEL_QUERY_PAYLOAD_META_HPP
#define IROHA_SHARED_MODEL_QUERY_PAYLOAD_META_HPP

#include "interfaces/base/primitive.hpp"

#ifndef DISABLE_BACKWARD
#include "model/query_payload_meta.hpp"
#endif

namespace shared_model {
  namespace interface {

    /**
     * Class QueryPayloadMeta provides metadata of query payload
     * General note: this class is container for queries but not a base class.
     */
    class QueryPayloadMeta : public PRIMITIVE(QueryPayloadMeta) {
     public:
      /**
       * @return id of query creator
       */
      virtual const types::AccountIdType &creatorAccountId() const = 0;

      /**
       * Query counter - incremental variable reflect for number of sent to
       * system queries plus 1. Required for preventing replay attacks.
       * @return attached query counter
       */
      virtual types::CounterType queryCounter() const = 0;

      /**
       * @return time of creation
       */
      virtual types::TimestampType createdTime() const = 0;

#ifndef DISABLE_BACKWARD
      OldModelType *makeOldModel() const override {
        auto *old_model = new OldModelType();
        old_model->creator_account_id = creatorAccountId();
        old_model->query_counter = queryCounter();
        // signature related
        old_model->created_ts = createdTime();
        return old_model;
      }
#endif
      bool operator==(const ModelType &rhs) const override {
        return creatorAccountId() == rhs.creatorAccountId()
            && queryCounter() == rhs.queryCounter()
            && createdTime() == rhs.createdTime();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_QUERY_PAYLOAD_META_HPP
