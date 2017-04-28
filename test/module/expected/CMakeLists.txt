########################################################################################
# Expected
########################################################################################

add_executable(expected_test expected_test.cpp)
target_link_libraries(expected_test
  gtest
  expected
  flatbuffers
)

add_test(
  NAME expected_test
  COMMAND $<TARGET_FILE:expected_test>
)