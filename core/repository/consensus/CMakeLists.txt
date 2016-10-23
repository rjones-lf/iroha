SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

link_directories(
  ${PROJECT_BINARY_DIR}/lib
)

ADD_LIBRARY(event_repository STATIC event_repository.cpp)
target_link_libraries(event_repository
  world_state_repo_with_level_db
)  

ADD_LIBRARY(merkle_transaction_repository STATIC merkle_transaction_repository.cpp)
target_link_libraries(merkle_transaction_repository  
  world_state_repo_with_level_db
)  

ADD_LIBRARY(unconfirmed_transaction_repository STATIC unconfirmed_transaction_repository.cpp)
target_link_libraries(unconfirmed_transaction_repository
  world_state_repo_with_level_db
)  
