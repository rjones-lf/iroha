/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_HEIGHT_VALIDATOR_HPP
#define IROHA_HEIGHT_VALIDATOR_HPP

#include <boost/format.hpp>
#include "interfaces/common_objects/types.hpp"
#include "validators/answer.hpp"

namespace shared_model {
  namespace validation {

  class HeightValidator {
   protected:
    virtual void validateHeight(ReasonsGroupType &reason,
                        const interface::types::HeightType &height) const {
      if (height <= 0) {
        auto message =
            (boost::format("Height should be > 0, passed value: %d") % height)
                .str();
        reason.second.push_back(message);
      }
    }
  };

  }
}  // namespace shared_model

#endif  // IROHA_HEIGHT_VALIDATOR_HPP
