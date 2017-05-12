ADD_LIBRARY(repository STATIC
    respository.cpp
)

target_link_libraries(repository
    ametsuchi
    flatbuffer_service
    connection_with_grpc_flatbuffer
)