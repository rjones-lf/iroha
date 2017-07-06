# Hash Test
AddTest(hash_test hash_test.cpp)
target_link_libraries(hash_test crypto ssl)

# Base64 Test
AddTest(base64_test base64_test.cpp)
target_link_libraries(base64_test crypto ssl)

# Singature Test
AddTest(signature_test signature_test.cpp)
target_link_libraries(signature_test crypto)