SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(MESSAGE_PACK_PATH ${PROJECT_SOURCE_DIR}/core/vendor/msgpack-c)
SET(LEVELDB_PATH ${PROJECT_SOURCE_DIR}/core/vendor/leveldb)

include_directories(
  ${LEVELDB_PATH}/include
  ${MESSAGE_PACK_PATH}/include
)

link_directories(
  ${MESSAGE_PACK_PATH}
  ${LEVELDB_PATH}/out-static
)

ADD_LIBRARY(merkle_transaction_repository STATIC merkle_transaction_repository.cpp)
target_link_libraries(merkle_transaction_repository
  msgpackc
  snappy
  leveldb
  exception
)

ADD_LIBRARY(unconfirmed_transaction_repository STATIC unconfirmed_transaction_repository.cpp)
target_link_libraries(unconfirmed_transaction_repository
  msgpackc
  snappy
  leveldb
  exception
)  
