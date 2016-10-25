SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_LIBRARY(sumeragi STATIC
  sumeragi.cpp
)
target_link_libraries(sumeragi
  signature
  event_repository
  merkle_transaction_repository
)