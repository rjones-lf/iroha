# Use for remove "error: cast from 'const iroha::Signature*' to 'flatbuffers::uoffset_t {aka unsigned int}' loses precision"
set(CMAKE_CXX_FLAGS "-g -std=c++1y -Wall -fPIC -fpermissive")

ADD_LIBRARY(http_client STATIC
    http_client.cpp
)

target_link_libraries(http_client
    ssl
    crypto
    logger
    json
)

ADD_LIBRARY(flatbuffer_service STATIC
    flatbuffer_service.cpp
)

target_link_libraries(flatbuffer_service
    logger
    expected
    flatbuffers
    grpc
    exception
    membership_service
    signature
    base64
    hash
)
