# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(mst_transport
        impl/mst_transport_grpc.cpp
        )

target_link_libraries(mst_transport
        mst_grpc
        mst_state
        shared_model_interfaces_factories
        )
