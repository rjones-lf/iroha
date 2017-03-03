add_library(create_objects_helper STATIC
  create_objects_helper.cpp
)

target_link_libraries(create_objects_helper
  event_with_grpc
)
