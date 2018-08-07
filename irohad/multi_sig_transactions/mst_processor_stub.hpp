/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef IROHA_MST_PROCESSOR_STUB_HPP
#define IROHA_MST_PROCESSOR_STUB_HPP

#include "multi_sig_transactions/mst_processor.hpp"

namespace iroha {
  class MstProcessorStub : public MstProcessor {
    auto propagateBatchImpl(const DataType &batch)
        -> decltype(propagateBatch(batch)) override;

    auto onStateUpdateImpl() const -> decltype(onStateUpdate()) override;

    auto onPreparedBatchesImpl() const
        -> decltype(onPreparedBatches()) override;

    auto onExpiredBatchesImpl() const -> decltype(onExpiredBatches()) override;
  };

}  // namespace iroha

#endif  // IROHA_MST_PROCESSOR_STUB_HPP
