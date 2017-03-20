###################################################
# java data structure
###################################################
add_library(java_data_structure STATIC
  java_data_structure.cpp
)

target_link_libraries(java_data_structure
  transaction_builder
  exception
  jni
  config_manager
  peer_service
)

###################################################
# java virtual machine
###################################################
add_library(java_virtual_machine STATIC
  java_virtual_machine.cpp
)

target_link_libraries(java_virtual_machine
  java_data_structure
  json
)
