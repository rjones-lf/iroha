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

#include "datetime/time.hpp"

using namespace std::chrono_literals;

namespace iroha {
  namespace validation {
    StatelessValidatorImpl::StatelessValidatorImpl(
        std::shared_ptr<model::ModelCryptoProvider> crypto_provider)
        : crypto_provider_(std::move(crypto_provider)),
          log_(logger::log("SLV")) {}

    bool StatelessValidatorImpl::validate(
        const model::Transaction &transaction) const {
      // signatures are correct
      if (!crypto_provider_->verify(transaction)) {
        log_->warn("crypto verification broken");
        return false;
      }

      // time between creation and validation of tx
      ts64_t now = time::now();

      // tx is not sent from future
      // todo make future gap for passing timestamp, like with old timestamps
      if (now < transaction.created_ts) {
        log_->warn("timestamp broken: send from future ({}, now {})",
                   transaction.created_ts,
                   now);
        return false;
      }

      if (now - transaction.created_ts > MAX_DELAY) {
        log_->warn("timestamp broken: too old ({}, now {})",
                   transaction.created_ts,
                   now);
        return false;
      }

      log_->info("transaction validated");
      return true;
    }

    bool StatelessValidatorImpl::validate(
        std::shared_ptr<const model::Query> query) const {
      // signatures are correct
      if (!crypto_provider_->verify(query)) {
        log_->warn("crypto verification is broken");
        return false;
      }

      // time between creation and validation of the query
      ts64_t now = time::now();

      // query is not sent from future
      // todo make future gap for passing timestamp, like with old timestamps
      if (now < query->created_ts) {
        log_->warn("bad timestamp: send from future: {}. Now {}",
                   query->created_ts, now);
        return false;
      }

      if (now - query->created_ts > MAX_DELAY) {
        log_->warn("bad timestamp: too old: {}. Now {}", query->created_ts,
                   now);
        return false;
      }

      log_->info("query validated");
      return true;
    }
  }
}
