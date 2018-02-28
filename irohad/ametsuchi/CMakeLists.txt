add_subdirectory(impl/flat_file)

add_library(ametsuchi
    impl/storage_impl.cpp
    impl/temporary_wsv_impl.cpp
    impl/mutable_storage_impl.cpp
    impl/postgres_wsv_query.cpp
    impl/postgres_wsv_command.cpp
    impl/peer_query_wsv.cpp
    impl/postgres_block_query.cpp
    impl/postgres_block_index.cpp
    impl/postgres_ordering_service_persistent_state.cpp
    )

target_link_libraries(ametsuchi
    json_model_converters
    logger
    rxcpp
    optional
    pqxx
    libs_common
    command_execution
    boost
    flat_file
    )
