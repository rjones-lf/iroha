# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

add_subdirectory(backend_proto)
add_subdirectory(bindings)
add_subdirectory(converters)
add_subdirectory(builders)
add_subdirectory(validators)
add_subdirectory(cryptography)

AddTest(lazy_test
    lazy_test.cpp
    )
target_link_libraries(lazy_test
    boost
    )

AddTest(reference_holder_test
    reference_holder_test.cpp
    )
target_link_libraries(reference_holder_test
    boost
    )

AddTest(interface_test
    interface_test.cpp
    )
target_link_libraries(interface_test
    shared_model_default_builders
    logger
    )
