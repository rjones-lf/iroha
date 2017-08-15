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

#include "validation/impl/stateless_validator_impl.hpp"
#include <chrono>
#include <utility>

namespace iroha {
  namespace validation {
    StatelessValidatorImpl::StatelessValidatorImpl(
        std::shared_ptr<model::ModelCryptoProvider> crypto_provider)
        : crypto_provider_(std::move(crypto_provider)) {
      log_ = logger::log("SLV");
    }

    bool StatelessValidatorImpl::validate(
        const model::Transaction& transaction) const {
      // signatures are correct
      if (!crypto_provider_->verify(transaction)){
        log_->warn("crypto verification broken");
        return false;
      }

      // time between creation and validation of tx
      uint64_t now = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch()).count());

      if (now - transaction.created_ts > MAX_DELAY) {
        log_->warn("timestamp broken: too old");
        return false;
      }

      // tx is not sent from future
      // todo make future gap for passing timestamp, like with old timestamps
      if (now < transaction.created_ts) {
        log_->warn("timestamp broken: send from future");
        return false;
      }
      log_->info("transaction validated");
      return true;
    }

    bool StatelessValidatorImpl::validate(std::shared_ptr<const model::Query> query) const {
      // signatures are correct
      if (!crypto_provider_->verify(query)){
        log_->warn("crypto verification broken");
        return false;
      }

      // time between creation and validation of the query
      uint64_t now = static_cast<uint64_t>(
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch()).count());

      // query is not sent from future
      // todo make future gap for passing timestamp, like with old timestamps
      if (now < query->created_ts) {
        log_->warn("timestamp broken: send from future. Now {}", now);
        return false;
      }

      if (now - query->created_ts > MAX_DELAY) {
        log_->warn("timestamp broken: too old. Now {}", now);
        return false;
      }


      log_->info("query validated");
      return true;
    }
  }
}
