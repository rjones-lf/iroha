# Ametsuchi Test
add_executable(ametsuchi_test ametsuchi.cc)
target_link_libraries(ametsuchi_test
  gtest
  spdlog
  ametsuchi
)
add_test(
  NAME ametsuchi_test
  COMMAND $<TARGET_FILE:ametsuchi_test>
)
