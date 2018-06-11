/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "interfaces/query_responses/account_asset_response.hpp"
#include "utils/string_builder.hpp"

namespace shared_model {
  namespace interface {

    std::string AccountAssetResponse::toString() const {
      return detail::PrettyStringBuilder()
          .init("AccountAssetResponse")
          .append(accountAsset().toString())
          .finalize();
    }

    bool AccountAssetResponse::operator==(const ModelType &rhs) const {
      return accountAsset() == rhs.accountAsset();
    }

  }  // namespace interface
}  // namespace shared_model
