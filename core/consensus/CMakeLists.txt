SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

ADD_LIBRARY(sumeragi STATIC
  sumeragi.cpp
  consensus_event.cpp
)
target_link_libraries(sumeragi
  event_repository
  crypto
  validator
  merkle_transaction_repository
  unconfirmed_transaction_repository
)