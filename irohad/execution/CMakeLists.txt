#
#Copyright Soramitsu Co., Ltd. All Rights Reserved.
#SPDX-License-Identifier: Apache-2.0
#

add_library(common_execution
        impl/common_executor.cpp
        )
target_link_libraries(common_execution
        rxcpp
        boost
        shared_model_proto_backend
        )

add_library(command_execution
        impl/command_executor.cpp
        )
target_link_libraries(command_execution
        ${Boost_LIBRARIES}
        logger
        common_execution
        rxcpp
        shared_model_default_builders
        shared_model_amount_utils
        )

add_library(query_execution
        impl/query_execution_impl.cpp
        )

target_link_libraries(query_execution
        rxcpp
        shared_model_default_builders
        common_execution
        )
