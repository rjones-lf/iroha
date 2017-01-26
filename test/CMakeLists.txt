set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/test_bin)
set(GTEST_INCLUDE_DIR ${PROJECT_BINARY_DIR}/gtest/src/googletest/googletest/include)

add_subdirectory(crypto)
#add_subdirectory(smart_contract)
#add_subdirectory(infra/repository)
add_subdirectory(infra/protobuf)
add_subdirectory(infra/service)
## TODO: connection_with_grpc requires 2 arguments. 
## Set default arguments, to be able to run just ./connection_with_grpc
#add_subdirectory(connection)
add_subdirectory(consensus)
add_subdirectory(vendor)
