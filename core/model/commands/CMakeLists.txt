SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_LIBRARY(commands STATIC
  add.cpp
  transfer.cpp
)

target_link_libraries(commands
  objects
)