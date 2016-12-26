SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_library(event_repository STATIC
    transaction_repository.cpp
)
target_link_libraries(event_repository
    world_state_repo_with_level_db
    event_with_grpc
    logger
)
#add_library(merkle_transaction_repository STATIC
#    merkle_transaction_repository.cpp
#)
#target_link_libraries(merkle_transaction_repository
#    hash
#    world_state_repo_with_level_db
#)