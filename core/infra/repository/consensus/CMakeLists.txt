SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)


SET(LEVELDB_PATH ${PROJECT_SOURCE_DIR}/core/vendor/leveldb)

include_directories(
  ${LEVELDB_PATH}/include
)

link_directories(
  ${LEVELDB_PATH}/out-static
)

ADD_LIBRARY(merkle_transaction_repository STATIC merkle_transaction_repository.cpp)
target_link_libraries(merkle_transaction_repository
  snappy
  leveldb
  exception
)  

