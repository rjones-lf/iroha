#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#

addtest(model_query_builder_test
    model_query_builder_test.cpp
    )

target_link_libraries(model_query_builder_test
    bindings
    )

addtest(model_crypto_test
    model_crypto_test.cpp
    )

target_link_libraries(model_crypto_test
    bindings
    )

if (SWIG_JAVA)
  get_property(SWIG_BUILD_DIR GLOBAL PROPERTY SWIG_BUILD_DIR)
  find_package(Java REQUIRED)

  get_target_property(PROTOC_EXEC_CMAKE protoc IMPORTED_LOCATION)
  file(TO_NATIVE_PATH ${SWIG_BUILD_DIR} SWIG_LIB_DIR)
  file(TO_NATIVE_PATH ${PROTOC_EXEC_CMAKE} PROTOC_EXEC)

  if (MSVC)
    set(SEPARATOR $<SEMICOLON>)
  else ()
    set(SEPARATOR :)
  endif()

  if(CMAKE_GENERATOR MATCHES "Visual Studio")
    set(SWIG_LIB_DIR "${SWIG_LIB_DIR}\\$<CONFIG>")
  endif()

  add_test(NAME java_builders_test
    COMMAND gradle test "-PSWIG_BUILD_DIR=${SWIG_BUILD_DIR}"
    "-PSCHEMA_DIR=${PROJECT_SOURCE_DIR}/shared_model/schema"
    "-PPROTOBUF_PROTOC_EXECUTABLE=${PROTOC_EXEC}"
    "-PPATH_DIRS=$ENV{PATH}${SEPARATOR}${SWIG_LIB_DIR}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
  set_tests_properties(java_builders_test
    PROPERTIES DEPENDS builders)
endif()
