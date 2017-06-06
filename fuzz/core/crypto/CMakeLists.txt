
add_executable(hash_fuzz hash_fuzz.cpp)
add_dependencies(hash_fuzz libFuzzer)
target_link_libraries(hash_fuzz fuzzer hash crypto)
