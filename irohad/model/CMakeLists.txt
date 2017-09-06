add_subdirectory(generators)
add_subdirectory(converters)

add_library(model
    model_crypto_provider_impl.cpp
    model_hash_provider_impl.cpp
    impl/model_operators.cpp
    impl/query_execution.cpp
    )
target_link_libraries(model
    hash
    optional
    rxcpp
    logger
    )


add_library(model_registrations INTERFACE)
target_include_directories(model_registrations INTERFACE
    registration
    )
target_link_libraries(model_registrations INTERFACE
    model
    )

add_library(command_execution
    execution/impl/command_executor.cpp
    execution/impl/command_executor_factory.cpp
    )
target_link_libraries(command_execution
    ametsuchi
    )
