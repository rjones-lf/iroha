SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_LIBRARY(commands STATIC
  add.cpp
  remove.cpp
  transfer.cpp
  update.cpp
)

target_link_libraries(commands
  objects
  domain_repository
  logger
)
