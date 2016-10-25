SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

SET(CMAKE_CXX_FLAGS_DEBUG "-g")

SET(KECCAK_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/KeccakCodePackage")
SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")
SET(AERON_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/Aeron")

include_directories(
    ${GTEST_INCLUDE_DIR}
)
link_directories(
    ${PROJECT_BINARY_DIR}/lib
    ${ED25519_PATH}
    ${KECCAK_PATH}/bin/generic64
    ${PROJECT_SOURCE_DIR}/core/infra/crypto
    ${AERON_PATH}/cppbuild/Debug/lib
)

add_executable(sumeragi_test
    sumeragi_test.cpp
)
target_link_libraries(sumeragi_test
    event_repository
    transfer_transaction
    sumeragi
    connection_with_aeron
    peer_service
    ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a pthread)
add_test(
    NAME sumeragi_test
    COMMAND $<TARGET_FILE:sumeragi_test>
)
