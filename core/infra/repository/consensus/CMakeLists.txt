ADD_LIBRARY(merkle_transaction_repository STATIC merkle_transaction_repository.cpp)

target_link_libraries(merkle_transaction_repository
  world_state_repo_with_level_db
  exception
)  

