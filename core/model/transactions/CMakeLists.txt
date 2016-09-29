set(CMAKE_BUILD_TYPE Debug)

# This is infra knowledge...
SET(MESSAGE_PACK_PATH ${PROJECT_SOURCE_DIR}/core/vendor/msgpack-c)
include_directories(${MESSAGE_PACK_PATH}/include)

ADD_LIBRARY(transfer_transaction STATIC
  transfer_transaction.cpp
)
