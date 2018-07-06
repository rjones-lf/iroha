/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/query_responses/query_response.hpp"
#include "utils/visitor_apply_for_all.hpp"

namespace shared_model {
  namespace interface {

    std::string QueryResponse::toString() const {
      return boost::apply_visitor(detail::ToStringVisitor(), get());
    }

    bool QueryResponse::operator==(const ModelType &rhs) const {
      return queryHash() == rhs.queryHash() and get() == rhs.get();
    }

  }  // namespace interface
}  // namespace shared_model
