add_subdirectory(impl)


add_library(ametsuchi
    impl/storage_impl.cpp
    impl/temporary_wsv_impl.cpp
    impl/mutable_storage_impl.cpp
    impl/postgres_wsv_query.cpp
    impl/postgres_wsv_command.cpp
    impl/peer_query_wsv.cpp
    impl/redis_block_query.cpp
    impl/redis_block_index.cpp
    )

target_link_libraries(ametsuchi
    json_model_converters
    logger
    rxcpp
    optional
    pqxx
    cpp_redis
    libs_common
    command_execution
    boost
    block_storage_nudb
    )
