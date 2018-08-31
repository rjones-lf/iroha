add_library(ametsuchi
    impl/flat_file/flat_file.cpp
    impl/storage_impl.cpp
    impl/temporary_wsv_impl.cpp
    impl/mutable_storage_impl.cpp
    impl/postgres_wsv_query.cpp
    impl/postgres_wsv_command.cpp
    impl/peer_query_wsv.cpp
    impl/postgres_block_query.cpp
    impl/postgres_command_executor.cpp
    impl/postgres_block_index.cpp
    impl/postgres_ordering_service_persistent_state.cpp
    impl/wsv_restorer_impl.cpp
    impl/postgres_options.cpp
    )

target_link_libraries(ametsuchi
    logger
    rxcpp
    libs_common
    common
    query_execution
    shared_model_interfaces
    shared_model_stateless_validation
    SOCI::core
    SOCI::postgresql
    )

target_compile_definitions(ametsuchi
    PRIVATE SOCI_USE_BOOST HAVE_BOOST
    )
