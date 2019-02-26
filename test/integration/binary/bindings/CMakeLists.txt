#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

add_library(bindings
        model_crypto.cpp
        )

target_link_libraries(bindings
        shared_model_cryptography
        generator
        common
        )