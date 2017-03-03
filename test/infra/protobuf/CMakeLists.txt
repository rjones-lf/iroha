add_executable(convertor_test
    convertor_test.cpp
)
target_link_libraries(convertor_test
   signature
   datetime
   hash
   base64
   world_state_repo_with_level_db
   event_repository
   grpc++
   grpc++_reflection
   protobuf
   dl
   gtest
   
   pthread
)
add_test(
    NAME convertor_test
    COMMAND $<TARGET_FILE:convertor_test>
)
