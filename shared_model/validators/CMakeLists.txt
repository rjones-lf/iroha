# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

add_library(shared_model_stateless_validation
        field_validator.cpp
        transactions_collection/transactions_collection_validator.cpp
        transactions_collection/batch_order_validator.cpp
        )

target_link_libraries(shared_model_stateless_validation
        schema
        shared_model_interfaces
        )
