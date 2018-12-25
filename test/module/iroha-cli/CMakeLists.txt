#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

addtest(client_test client_test.cpp)
target_link_libraries(client_test
    client
    processors
    server_runner
    )
target_include_directories(client_test PUBLIC
    ${PROJECT_SOURCE_DIR}/iroha-cli
    )
