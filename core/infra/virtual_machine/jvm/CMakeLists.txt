###################################################
# java data structure
# 
# link
#   - create_objects_helper
###################################################
add_library(java_data_structure STATIC
  java_data_structure.cpp
)

target_link_libraries(java_data_structure
  create_objects_helper
  exception
  jvm
  config_manager
)

###################################################
# java virtual machine
#
# link
#   - java_virtual_machine
#   - java_data_structure
###################################################
add_library(java_virtual_machine STATIC
  java_virtual_machine.cpp
)

target_link_libraries(java_virtual_machine
  java_data_structure
  json
)
