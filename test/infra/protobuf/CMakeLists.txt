set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)
SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")
set(GTEST_INCLUDE_DIR ${PROJECT_BINARY_DIR}/gtest/src/googletest/googletest/include)

include_directories(
    ${GTEST_INCLUDE_DIR}
)
link_directories(
    ${ED25519_PATH}/lib
    ${PROJECT_BINARY_DIR}/lib
    ${PROJECT_SOURCE_DIR}/core/infra/crypto
)

add_executable(convertor_test
    convertor_test.cpp
)
target_link_libraries(convertor_test
   convertor
   signature
   datetime
   objects

   hash
   base64

   world_state_repo_with_level_db
   event_repository

   grpc++
   grpc++_reflection
   protobuf
   pthread
   dl

   ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a pthread
)
add_test(
    NAME convertor_test
    COMMAND $<TARGET_FILE:convertor_test>
)
