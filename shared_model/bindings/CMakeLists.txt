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


add_library(bindings
        model_transaction_builder.cpp
        model_query_builder.cpp
        model_crypto.cpp
        )

target_link_libraries(bindings
        shared_model_proto_backend
        shared_model_cryptography
        generator
        shared_model_stateless_validation
        iroha_amount
        )



if (SWIG_PYTHON OR SWIG_JAVA OR SWIG_CSHARP OR SWIG_NODE)
    find_package(swig REQUIRED)
    include(${SWIG_USE_FILE})

    set(CMAKE_SWIG_FLAGS "")
    if (SHARED_MODEL_DISABLE_COMPATIBILITY)
        set(CMAKE_SWIG_FLAGS "-DDISABLE_BACKWARD")
    endif()
    set_source_files_properties(bindings.i PROPERTIES CPLUSPLUS ON)
    set_property(GLOBAL PROPERTY SWIG_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

    add_dependencies(bindings swig)

    macro (myswig_add_library target)
        swig_add_library(${ARGV})
        # get internal dependencies
        get_target_property(dependencies
            ${SWIG_MODULE_${target}_REAL_NAME}
            MANUALLY_ADDED_DEPENDENCIES)
        if (dependencies)
            # add external project dependency on internal targets
            foreach (dependency IN LISTS dependencies)
                add_dependencies(${dependency} swig)
            endforeach()
        endif()
    endmacro()
endif()

if (SWIG_PYTHON)
    if(SUPPORT_PYTHON2)
        set(PYTHON_VER 2.7)
    else()
        set(PYTHON_VER 3.5)
    endif()

    if(CMAKE_GENERATOR MATCHES "Visual Studio")
        find_package(PythonInterp ${PYTHON_VER} REQUIRED)
    endif()
    find_package(PythonLibs ${PYTHON_VER} REQUIRED)

    if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
        set(MAC_OPTS "-flat_namespace -undefined suppress")
    endif()

    myswig_add_library(iroha LANGUAGE python SOURCES bindings.i)
    swig_link_libraries(iroha ${PYTHON_LIBRARIES} bindings ${MAC_OPTS})
    add_custom_target(irohapy DEPENDS ${SWIG_MODULE_iroha_REAL_NAME})
    # path to where Python.h is found
    target_include_directories(${SWIG_MODULE_iroha_REAL_NAME} PUBLIC
        ${PYTHON_INCLUDE_DIRS}
        )
endif()

if (SWIG_JAVA)
    find_package(JNI REQUIRED)

    myswig_add_library(irohajava LANGUAGE java SOURCES bindings.i)
    swig_link_libraries(irohajava ${Java_LIBRARIES} bindings)
    # the include path to jni.h and jni_md.h
    target_include_directories(${SWIG_MODULE_irohajava_REAL_NAME} PUBLIC
        ${JAVA_INCLUDE_PATH}
        ${JAVA_INCLUDE_PATH2}
        )
endif()

if (SWIG_CSHARP)
    myswig_add_library(libirohacs LANGUAGE csharp SOURCES bindings.i)
    swig_link_libraries(libirohacs bindings)
    add_custom_target(irohacs DEPENDS ${SWIG_MODULE_libirohacs_REAL_NAME})
endif()

if (SWIG_NODE)
    find_package (nodejs REQUIRED)

    set (V8_VERSION_HEX 0x0${V8_VERSION_MAJOR}${V8_VERSION_MINOR}${V8_VERSION_PATCH})
    string (LENGTH "${V8_VERSION_HEX}" V8_VERSION_HEX_length)
    while (V8_VERSION_HEX_length LESS 8)
        set (V8_VERSION_HEX "${V8_VERSION_HEX}0")
        message (STATUS "INFO - Padded V8 version to match SWIG format")
        string (LENGTH "${V8_VERSION_HEX}" V8_VERSION_HEX_length)
    endwhile ()

    if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
        set(MAC_OPTS "-flat_namespace -undefined suppress")
    endif()

    set_property(SOURCE bindings.i PROPERTY SWIG_FLAGS "-node" "-DV8_VERSION=${V8_VERSION_HEX}")

    # Build SWIG library always statically for the subsequent assembly by GYP
    myswig_add_library(irohanode
        TYPE STATIC 
        LANGUAGE javascript 
        SOURCES bindings.i
        )
    set_target_properties(irohanode PROPERTIES PREFIX ${CMAKE_STATIC_LIBRARY_PREFIX})
    target_link_libraries(irohanode bindings ${MAC_OPTS})
    target_include_directories(${SWIG_MODULE_irohanode_REAL_NAME} PUBLIC
        ${NODEJS_INCLUDE_DIRS}
        )
endif()
