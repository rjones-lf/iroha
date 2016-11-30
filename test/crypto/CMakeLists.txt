
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)

SET(CRYPTO_PATH  "${PROJECT_SOURCE_DIR}/core/crypto")
SET(KECCAK_PATH  "${PROJECT_SOURCE_DIR}/core/vendor/KeccakCodePackage")
SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")

include_directories(
  ${CRYPTO_PATH}
  ${GTEST_INCLUDE_DIR}  
  ${PROJECT_SOURCE_DIR}/core/vendor/ed25519/src
  ${KECCAK_PATH}/bin/generic64/libkeccak.a.headers  
)
link_directories(
  ${ED25519_PATH}/lib
  ${PROJECT_BINARY_DIR}/lib
  ${KECCAK_PATH}/bin/generic64
  # Bad solution
  ${PROJECT_SOURCE_DIR}/core/infra/crypto
)

# Hash Test
add_executable(hash_test
  hash_test.cpp
)
target_link_libraries(hash_test hash
  keccak
  crypto
  ssl
  ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a pthread
)
add_test(
  NAME hash_test
  COMMAND $<TARGET_FILE:hash_test>
)

# Base64 Test
add_executable(base64_test base64_test.cpp)
target_link_libraries(base64_test base64
  ssl
  crypto
  ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a pthread)
add_test(
  NAME base64_test
  COMMAND $<TARGET_FILE:base64_test>
)

# Singature Test
add_executable(signature_test signature_test.cpp)
target_link_libraries(signature_test signature
  base64
  hash
  ed25519
  ${GTEST_LIBS_DIR}/libgtest.a ${GTEST_LIBS_DIR}/libgtest_main.a pthread)
add_test(
  NAME signature_test
  COMMAND $<TARGET_FILE:signature_test>
)
