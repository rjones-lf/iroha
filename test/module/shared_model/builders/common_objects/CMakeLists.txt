#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

addtest(peer_builder_test
        peer_builder_test.cpp
        )

target_link_libraries(peer_builder_test
        shared_model_proto_builders
        shared_model_stateless_validation
        )

addtest(signature_builder_test
        signature_builder_test.cpp
        )

target_link_libraries(signature_builder_test
        shared_model_proto_builders
        shared_model_stateless_validation
        )

