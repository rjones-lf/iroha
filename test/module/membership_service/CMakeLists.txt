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

add_executable(peer_service_to_issue_transaction_test peer_service_to_issue_transaction_test.cpp)
target_link_libraries(peer_service_to_issue_transaction_test
  gtest
  endpoint_fbs
  membership_service
  flatbuffer_service
  connection_with_grpc_flatbuffer
)
add_test(
  NAME peer_service_to_issue_transaction_test
  COMMAND $<TARGET_FILE:peer_service_to_issue_transaction_test>
)