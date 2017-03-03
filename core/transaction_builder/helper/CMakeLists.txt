SET(CMAKE_CXX_FLAGS "-g -Wall -std=c++1y -fPIC")

link_directories(
  ${PROJECT_BINARY_DIR}/lib
)

add_library(create_objects_helper STATIC
  create_objects_helper.cpp
)

target_link_libraries(create_objects_helper
  event_with_grpc
)
