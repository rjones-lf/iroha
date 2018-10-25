# Copyright 2017 Soramitsu Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

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

if (SWIG_NODE)
  find_package (nodejs REQUIRED)
  add_test(NAME javascript_tests
    COMMAND npm run build-and-test
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/shared_model/packages/javascript)
endif()
