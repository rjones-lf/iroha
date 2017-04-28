

# Use for remove "error: cast from 'const iroha::Signature*' to 'flatbuffers::uoffset_t {aka unsigned int}' loses precision"
set(CMAKE_CXX_FLAGS "-g -std=c++1y -Wall -fPIC -fpermissive")

ADD_LIBRARY(sumeragi STATIC
  sumeragi.cpp
)

target_link_libraries(sumeragi
  config_manager
  connection_with_grpc_flatbuffer
  flatbuffer_service
  signature
  thread_pool
  timer
)
