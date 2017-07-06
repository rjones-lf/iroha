
add_executable(hash256_fuzz hash256_fuzz.cpp)
add_dependencies(hash256_fuzz libFuzzer)
target_link_libraries(hash256_fuzz fuzzer hash)

add_executable(hash512_fuzz hash512_fuzz.cpp)
add_dependencies(hash512_fuzz libFuzzer)
target_link_libraries(hash512_fuzz fuzzer hash)

add_executable(base64enc_fuzz base64enc_fuzz.cpp)
add_dependencies(base64enc_fuzz libFuzzer)
target_link_libraries(base64enc_fuzz fuzzer base64)
