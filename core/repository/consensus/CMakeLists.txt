add_library(merkle_transaction_repository STATIC
    merkle_transaction_repository.cpp
)

target_link_libraries(merkle_transaction_repository
    hash
    world_state_repo_with_level_db
)
