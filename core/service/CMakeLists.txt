
ADD_LIBRARY(peer_service STATIC peer_service.cpp)
target_link_libraries(peer_service
  event_repository
  merkle_transaction_repository
  unconfirmed_transaction_repository
  exception
  # connect infra
  world_state_repo_with_level_db
)  
