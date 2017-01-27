add_subdirectory(objects)
add_subdirectory(commands)

ADD_LIBRARY(transaction STATIC
  transaction.cpp
)

target_link_libraries(transaction
  commands
  objects
  datetime
  hash
  logger
)