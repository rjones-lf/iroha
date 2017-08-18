# Interactive Client
add_library(interactive_cli
    impl/interactive_cli.cpp
    impl/interactive_common_cli.cpp
    impl/interactive_query_cli.cpp
    )
target_link_libraries(interactive_cli
    model
    parser
    )
target_include_directories(interactive_cli PUBLIC
    ${PROJECT_SOURCE_DIR}/iroha-cli
    )