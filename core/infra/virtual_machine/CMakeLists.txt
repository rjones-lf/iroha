add_subdirectory(jvm)

include_directories(${PROJECT_SOURCE_DIR}/core)

# Libraries
add_library(virtual_machine_with_java STATIC
  virtual_machine_with_java.cpp
)

# link
target_link_libraries(virtual_machine_with_java
  java_virtual_machine
  logger
  json
)
