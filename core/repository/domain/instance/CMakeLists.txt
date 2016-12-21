SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_LIBRARY(domain_repository STATIC
  asset_repository.cpp
  account_repository.cpp)
target_link_libraries(domain_repository
  exception
  # connect infra
  world_state_repo_with_level_db

    event_with_grpc
    grpc++
    grpc++_reflection
    protobuf
    pthread
    dl
)  
