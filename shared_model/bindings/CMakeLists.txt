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



if (SWIG_PYTHON OR SWIG_JAVA OR SWIG_CSHARP)
    find_package(swig REQUIRED)
    include(${SWIG_USE_FILE})

    set(CMAKE_SWIG_FLAGS "")
    if (SHARED_MODEL_DISABLE_COMPATIBILITY)
        set(CMAKE_SWIG_FLAGS "-DDISABLE_BACKWARD")
    endif()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR})
    set_source_files_properties(bindings.i PROPERTIES CPLUSPLUS ON)
    set_property(GLOBAL PROPERTY SWIG_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

    add_dependencies(bindings swig)
endif()

if (SWIG_PYTHON)
    if(SUPPORT_PYTHON2)
        find_package(PythonLibs 2.7 REQUIRED)
    else()
        find_package(PythonLibs 3.6 REQUIRED)
    endif()

    # path to where Python.h is found
    include_directories(${PYTHON_INCLUDE_DIRS})

    if (${CMAKE_SYSTEM_NAME} STREQUAL Darwin)
        set(MAC_OPTS "-flat_namespace -undefined suppress")
    endif()

    swig_add_library(iroha LANGUAGE python SOURCES bindings.i)
    swig_link_libraries(iroha ${Python_LIBRARIES} bindings ${MAC_OPTS})
    add_custom_target(irohapy DEPENDS ${SWIG_MODULE_iroha_REAL_NAME})

endif()

if (SWIG_JAVA)
    find_package(JNI REQUIRED)
    # the include path to jni.h
    include_directories(${JAVA_INCLUDE_PATH})
    # the include path to jni_md.h
    include_directories(${JAVA_INCLUDE_PATH2})

    swig_add_library(irohajava LANGUAGE java SOURCES bindings.i)
    swig_link_libraries(irohajava ${Java_LIBRARIES} bindings)
endif()

if (SWIG_CSHARP)
    swig_add_library(libirohacs LANGUAGE csharp SOURCES bindings.i)
    swig_link_libraries(libirohacs bindings)
    add_custom_target(irohacs DEPENDS ${SWIG_MODULE_libirohacs_REAL_NAME})
endif()
