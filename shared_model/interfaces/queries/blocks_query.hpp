/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_SHARED_MODEL_BLOCKS_QUERY_HPP
#define IROHA_SHARED_MODEL_BLOCKS_QUERY_HPP

#include <boost/variant.hpp>

#include "interfaces/base/primitive.hpp"
#include "interfaces/base/signable.hpp"
#include "interfaces/common_objects/types.hpp"

#ifndef DISABLE_BACKWARD
#include "model/queries/blocks_query.hpp"
#endif

namespace shared_model {
  namespace interface {

    /**
     * Class BlocksQuery provides container with one of concrete query available in
     * system.
     * General note: this class is container for queries but not a base class.
     */
    class BlocksQuery : public SIGNABLE(BlocksQuery) {
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

      // ------------------------| Primitive override |-------------------------

      std::string toString() const override {
        return detail::PrettyStringBuilder()
            .init("BlocksQuery")
            .append("creatorId", creatorAccountId())
            .append("queryCounter", std::to_string(queryCounter()))
            .append(Signable::toString())
            .finalize();
      }

#ifndef DISABLE_BACKWARD
      OldModelType *makeOldModel() const override {
        auto old_model = new OldModelType();
        old_model->creator_account_id = creatorAccountId();
        old_model->query_counter = queryCounter();
        // signature related
        old_model->created_ts = createdTime();
        std::for_each(signatures().begin(),
                      signatures().end(),
                      [&old_model](auto &signature_wrapper) {
                        // for_each cycle will assign last signature for old
                        // model. Also, if in new model absence at least one
                        // signature, this part will be worked correctly.
                        auto old_sig = signature_wrapper->makeOldModel();
                        old_model->signature = *old_sig;
                        delete old_sig;
                      });
        return old_model;
      }
#endif

      bool operator==(const ModelType &rhs) const override {
        return creatorAccountId() == rhs.creatorAccountId()
            && queryCounter() == rhs.queryCounter()
            && createdTime() == rhs.createdTime()
            && signatures() == rhs.signatures();
      }
    };
  }  // namespace interface
}  // namespace shared_model
#endif  // IROHA_SHARED_MODEL_BLOCKS_QUERY_HPP
