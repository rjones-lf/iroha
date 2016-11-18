SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# All domain instance is in libdomain.a
ADD_LIBRARY(commands STATIC
  add.cpp
  transfer.cpp
)

target_link_libraries(commands
  objects
)