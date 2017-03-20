SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

add_executable(iroha-main
  main.cpp
)
target_link_libraries(iroha-main
  sumeragi
  http_server_with_cappuccino
  izanami
  logger
)
