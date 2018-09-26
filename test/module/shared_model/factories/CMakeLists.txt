# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

addtest(transaction_batch_unsafe_factory_test transaction_batch_unsafe_factory_test.cpp)
target_link_libraries(transaction_batch_unsafe_factory_test
    shared_model_interfaces
    shared_model_proto_backend
    )
