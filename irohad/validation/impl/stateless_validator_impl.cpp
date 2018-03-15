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

#include "cryptography/crypto_provider/crypto_verifier.hpp"
#include "datetime/time.hpp"

#include "backend/protobuf/from_old_model.hpp"

using namespace std::chrono_literals;

namespace iroha {
  namespace validation {
    StatelessValidatorImpl::StatelessValidatorImpl(
        std::shared_ptr<shared_model::crypto::CryptoVerifier> crypto_verifier)
        : crypto_verifier_(std::move(crypto_verifier)) {
      log_ = logger::log("SLV");
    }

    const char *kCryptoVerificationFail = "Invalid signature";
    const char *kFutureTimestamp =
        "Timestamp is broken: sent from future {}. Now {}";
    const char *kOldTimestamp = "Timestamp broken: too old {}. Now {}";

    bool StatelessValidatorImpl::validate(
        const model::Transaction &old_transaction) const {
      // signatures are correct
      auto transaction = shared_model::proto::from_old(old_transaction);
      if (!crypto_verifier_->verify(transaction)) {
        log_->warn(kCryptoVerificationFail);
        return false;
      }

      // time between creation and validation of tx
      ts64_t now = time::now();

      // tx is not sent from future
      if (now < old_transaction.created_ts) {
        log_->warn(kFutureTimestamp, old_transaction.created_ts, now);
        return false;
      }

      if (now - old_transaction.created_ts > MAX_DELAY) {
        log_->warn(kOldTimestamp, old_transaction.created_ts, now);
        return false;
      }

      log_->info("transaction validated");
      return true;
    }

    bool StatelessValidatorImpl::validate(const model::Query &old_query) const {
      // signatures are correct
      auto query = shared_model::proto::from_old(std::make_shared<model::Query>(old_query));
      if (!crypto_verifier_->verify(query)) {
        log_->warn(kCryptoVerificationFail);
        return false;
      }

      // time between creation and validation of the query
      ts64_t now = time::now();

      // query is not sent from future
      // TODO 06/08/17 Muratov: make future gap for passing timestamp, like with
      // old timestamps IR-511 #goodfirstissue
      if (now < old_query.created_ts) {
        log_->warn(kFutureTimestamp, old_query.created_ts, now);
        return false;
      }

      if (now - old_query.created_ts > MAX_DELAY) {
        log_->warn(kOldTimestamp, old_query.created_ts, now);
        return false;
      }

      log_->info("query validated");
      return true;
    }
  }  // namespace validation
}  // namespace iroha
