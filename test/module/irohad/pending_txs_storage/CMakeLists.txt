#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

addtest(pending_txs_storage_test
    pending_txs_storage_test.cpp
    pending_txs_storage_fixture.cpp
    )

target_link_libraries(pending_txs_storage_test
    pending_txs_storage
    ## todo remove schema target, igor-egorov, IR-1512
    ## schema is included due to a dependency bug in shared_model_interfaces
    schema
    shared_model_interfaces
    shared_model_stateless_validation
    shared_model_cryptography
    )
