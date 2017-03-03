# Hash Test
add_executable(hash_test
  hash_test.cpp
)
target_link_libraries(hash_test
  crypto
  hash
  ssl
  gtest
)
add_test(
  NAME hash_test
  COMMAND $<TARGET_FILE:hash_test>
)


# Base64 Test
add_executable(base64_test base64_test.cpp)
target_link_libraries(base64_test 
  base64
  ssl
  crypto
  gtest
)
add_test(
  NAME base64_test
  COMMAND $<TARGET_FILE:base64_test>
)


# Singature Test
add_executable(signature_test signature_test.cpp)
target_link_libraries(signature_test 
  signature
  base64
  hash
  ed25519
  gtest
)
add_test(
  NAME signature_test
  COMMAND $<TARGET_FILE:signature_test>
)
