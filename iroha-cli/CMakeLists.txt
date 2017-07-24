cmake_minimum_required(VERSION 3.0)
PROJECT(iroha C CXX)

add_library(cli_client
    client.cpp
    )

target_link_libraries(cli_client
    endpoint)

add_executable(cli-main main.cpp)
target_link_libraries(cli-main gflags ed25519 cli_client)