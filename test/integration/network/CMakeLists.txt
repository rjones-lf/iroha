#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

addtest(mst_network_test mst_network_test.cpp)
target_link_libraries(mst_network_test
    integration_framework
    shared_model_proto_builders
    )
