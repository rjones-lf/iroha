# Copyright 2018 Soramitsu Co., Ltd.
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

cmake_minimum_required(VERSION 3.5.1)

PROJECT(shared_model C CXX)

SET(IROHA_SCHEMA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../schema")
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/../libs
        ${IROHA_SCHEMA_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
)

if (NOT IROHA_ROOT_PROJECT)
    SET(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
    SET(CMAKE_CXX_FLAGS "-std=c++1y -Wall")
    SET(CMAKE_CXX_FLAGS_RELEASE "-O3")
    SET(CMAKE_CXX_FLAGS_DEBUG   "-g -Wextra -Wno-unused-parameter -Wno-deprecated-declarations -O0")
    SET(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)

    SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Modules)

    add_definitions(-DDISABLE_BACKWARD)

    include(FeatureSummary)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/functions.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/dependencies.cmake)
        
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../schema schema)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libs libs)
endif()
add_library(old_model INTERFACE)

if (NOT SHARED_MODEL_DISABLE_COMPATIBILITY)
    target_link_libraries(old_model INTERFACE
        model
        )
endif()



add_subdirectory(backend)
add_subdirectory(bindings)
add_subdirectory(builders)
add_subdirectory(converters)
add_subdirectory(cryptography)
add_subdirectory(interfaces)
add_subdirectory(validators)
