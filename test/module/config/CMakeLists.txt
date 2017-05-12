# Config Utils test
add_executable(config_utils_test
  config_utils_test.cpp
)
target_link_libraries(config_utils_test
  gtest
  config_utils
  expected
  logger
)
add_test(
  NAME config_utils_test
  COMMAND $<TARGET_FILE:config_utils_test>
)
