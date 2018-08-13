/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/queries/query.hpp"
#include "utils/visitor_apply_for_all.hpp"

namespace shared_model {
  namespace interface {

    std::string Query::toString() const {
      return detail::PrettyStringBuilder()
          .init("Query")
          .append("creatorId", creatorAccountId())
          .append("queryCounter", std::to_string(queryCounter()))
          .append(Signable::toString())
          .append(boost::apply_visitor(detail::ToStringVisitor(), get()))
          .finalize();
    }

    bool Query::operator==(const ModelType &rhs) const {
      return this->get() == rhs.get();
    }

  }  // namespace interface
}  // namespace shared_model
