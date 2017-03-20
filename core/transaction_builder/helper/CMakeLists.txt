add_library(transaction_builder STATIC
  create_objects_helper.cpp
)

target_link_libraries(transaction_builder
  event_with_grpc
)
