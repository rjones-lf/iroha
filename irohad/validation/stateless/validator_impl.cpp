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

#include <chrono>
#include <model/model_crypto_provider_impl.hpp>
#include <validation/stateless/validator_impl.hpp>

namespace iroha {
  namespace validation {

    StatelessValidatorImpl::StatelessValidatorImpl(
        model::ModelCryptoProvider& crypto_provider)
        : crypto_provider_(crypto_provider) {
      max_delay_ =
          1000 * 3600 *
          24;  // Maximum delay between tx creation and validation is one day
    }

    bool StatelessValidatorImpl::validate(
        const model::Transaction& transaction) const {
      // signatures are correct
      {
        if (!crypto_provider_.verify(transaction)) return false;
      }

      // time between creation and validation of tx
      std::chrono::milliseconds now =
          std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::system_clock::now().time_since_epoch());
      {
        if (now.count() - transaction.created_ts > max_delay_) {
          return false;
        }
      }

      // tx is not sent from future
      {
        if (now.count() < transaction.created_ts) {
          return false;
        }
      }
      return true;
    }
  }  // namespace validation
}  // namespace iroha