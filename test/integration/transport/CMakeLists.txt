# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

addtest(ordering_gate_service_test ordering_gate_service_test.cpp)
target_link_libraries(ordering_gate_service_test
        ordering_service
        shared_model_stateless_validation
        shared_model_cryptography_model
        iroha_amount
    )
