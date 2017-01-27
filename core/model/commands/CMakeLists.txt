SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_LIBRARY(commands STATIC
  add.cpp
  transfer.cpp
  update.cpp
  remove.cpp
  batch.cpp
  unbatch.cpp
)

target_link_libraries(commands
  objects
  domain_repository
  logger
)