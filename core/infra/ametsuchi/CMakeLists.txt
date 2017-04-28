include_directories(
  # Library headers.
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${IROHA_SCHEMA_DIR}"
)

include(${PROJECT_SOURCE_DIR}/cmake/functions.cmake)

set(AMETSUCHI_INCLUDE
  include/ametsuchi/ametsuchi.h
  include/ametsuchi/tx_store.h
  include/ametsuchi/wsv.h
  include/ametsuchi/common.h
  include/ametsuchi/currency.h
  include/ametsuchi/exception.h
  include/ametsuchi/comparator.h
  include/ametsuchi/merkle_tree/narrow_merkle_tree.h
  include/ametsuchi/merkle_tree/circular_stack.h
  include/ametsuchi/merkle_tree/merkle_tree.h

  # needed to compile fbs automatically
  #${IROHA_SCHEMA_DIR}/account_generated.h
  #${IROHA_SCHEMA_DIR}/asset_generated.h
  #${IROHA_SCHEMA_DIR}/commands_generated.h
  #${IROHA_SCHEMA_DIR}/main_generated.h
  #${IROHA_SCHEMA_DIR}/primitives_generated.h
  #${IROHA_SCHEMA_DIR}/transaction_generated.h
  )

set(AMETSUCHI_SRC
  src/ametsuchi/ametsuchi.cc
  src/ametsuchi/tx_store.cc
  src/ametsuchi/wsv.cc
  src/ametsuchi/currency.cc
  src/ametsuchi/common.cc
  src/ametsuchi/merkle_tree/merkle_tree.cc
  )

# Library.
set(LIBAMETSUCHI_NAME ametsuchi)
add_library(${LIBAMETSUCHI_NAME} SHARED
  ${AMETSUCHI_INCLUDE}
  ${AMETSUCHI_SRC}
)
target_link_libraries(${LIBAMETSUCHI_NAME}
  spdlog
  LMDB
  flatbuffers
  keccak
  )
StrictMode(${LIBAMETSUCHI_NAME})


# Documentation.
if(DOCS)
  find_package(Doxygen)
  if(DOXYGEN_NOT_FOUND)
    message(FATAL_ERROR "Doxygen not found")
  endif()
  set(DOC_SOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
  set(DOC_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
  add_custom_target(
    doc ALL
    COMMAND ${DOXYGEN_EXECUTABLE}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Generating API documentation with Doxygen"
    VERBATIM
  )
endif(DOCS)



