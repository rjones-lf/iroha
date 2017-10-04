add_library(model_generators
    impl/signature_generator.cpp
    impl/block_generator.cpp
    impl/transaction_generator.cpp
    impl/command_generator.cpp
    impl/query_generator.cpp
    )

target_link_libraries(model_generators
    model
    optional
    generator
    keys_manager
    )
