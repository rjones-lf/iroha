cmake_minimum_required(VERSION 3.0)
PROJECT(iroha C CXX)

add_executable(cli-main main.cpp)
target_link_libraries(cli-main gflags ed25519)