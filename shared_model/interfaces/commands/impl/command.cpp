/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/commands/command.hpp"

namespace shared_model {
  namespace interface {

    std::string Command::toString() const {
      return boost::apply_visitor(detail::ToStringVisitor(), get());
    }

    bool Command::operator==(const ModelType &rhs) const {
      return this->get() == rhs.get();
    }

  }  // namespace interface
}  // namespace shared_model
