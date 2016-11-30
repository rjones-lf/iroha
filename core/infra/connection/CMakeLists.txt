
SET(CMAKE_CXX_FLAGS "-std=c++14 -L/usr/local/lib `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl")

#[[
SET(AERON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/Aeron")


include_directories(
  ${AERON_PATH}/aeron-client/src/main/cpp
)
link_directories(
    ${AERON_PATH}/cppbuild/Release/lib
    /usr/local/lib
)

ADD_LIBRARY(connection_with_aeron STATIC connection_with_aeron.cpp)
target_link_libraries(connection_with_aeron
  aeron_client
  logger
  peer_service_with_json
)
]]
# ]]

ADD_LIBRARY(connection_with_grpc STATIC
  connection_with_grpc.cpp
  connection.grpc.pb.cc
  connection.pb.cc    
)

target_link_libraries(connection_with_grpc
  grpc++
  grpc++_reflection
  protobuf
  pthread
  dl
  logger
  peer_service_with_json
)
