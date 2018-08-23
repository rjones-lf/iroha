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

#include "multi_sig_transactions/mst_processor.hpp"

namespace iroha {

  MstProcessor::MstProcessor() : log_(logger::log("MstProcessor")) {}

  void MstProcessor::propagateBatch(const DataType &batch) {
    this->propagateBatchImpl(batch);
  }

  rxcpp::observable<std::shared_ptr<MstState>> MstProcessor::onStateUpdate()
      const {
    return this->onStateUpdateImpl();
  }

  rxcpp::observable<DataType> MstProcessor::onPreparedBatches() const {
    return this->onPreparedBatchesImpl();
  }

  rxcpp::observable<DataType> MstProcessor::onExpiredBatches() const {
    return this->onExpiredBatchesImpl();
  }
}  // namespace iroha
