add_executable(java_data_structure_test
    java_data_structure_test.cpp
)
target_link_libraries(java_data_structure_test
  hash
  create_objects_helper
  virtual_machine_with_java
  gtest
)
add_test(
    NAME java_data_structure_test
    COMMAND $<TARGET_FILE:java_data_structure_test>
)
