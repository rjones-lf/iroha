########################################################################################
# PeerService
########################################################################################
add_executable(peer_service_test peer_service_test.cpp)
target_link_libraries(peer_service_test
  gtest
  membership_service
)
add_test(
  NAME peer_service_test
  COMMAND $<TARGET_FILE:peer_service_test>
)