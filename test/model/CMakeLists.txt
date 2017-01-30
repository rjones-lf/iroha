set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)
set(GTEST_INCLUDE_DIR ${PROJECT_BINARY_DIR}/gtest/src/googletest/googletest/include)

SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")

link_directories(
  ${PROJECT_BINARY_DIR}/lib
  ${ED25519_PATH}/lib
  ${PROJECT_SOURCE_DIR}/core/infra/crypto
)
  
include_directories(
  ${GTEST_INCLUDE_DIR}
)

add_executable(add_test add_test.cpp)
target_link_libraries(add_test
  peer_service_with_json  
  commands
  objects
  ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a pthread
)
  
add_test(
    NAME add_test
    COMMAND $<TARGET_FILE:add_test>
)
