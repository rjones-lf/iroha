#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_library(pending_txs_storage
    pending_txs_storage.cpp
    )

target_link_libraries(pending_txs_storage
    mst_state
    ## todo remove schema target, igor-egorov, IR-1512
    ## schema is included due to a dependency bug in shared_model_interfaces
    schema
    shared_model_interfaces
    )
