add_subdirectory(objects)

ADD_LIBRARY(transaction STATIC
  transaction.cpp
  objects/asset.cpp
  objects/object.cpp
  objects/simple_asset.cpp
  objects/peer.cpp
  commands/command.cpp
)

target_link_libraries(transaction
  datetime
  hash
  logger
)
