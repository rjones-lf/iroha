#
# Copyright Soramitsu Co., Ltd. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0
#


if (SWIG_PYTHON OR SWIG_JAVA)
  get_property(SWIG_BUILD_DIR GLOBAL PROPERTY SWIG_BUILD_DIR)
endif ()

if (SWIG_PYTHON)
  if (SUPPORT_PYTHON2)
    find_package(PythonInterp 2.7 REQUIRED)
  else ()
    find_package(PythonInterp 3.5 REQUIRED)
  endif ()

  # TODO, igor-egorov, 2018-06-27, IR-1481, move that foreach to a separate target
  foreach (item "block" "commands" "primitive" "queries")
    compile_proto_to_python("${item}.proto")
    list(APPEND PROTO_SWIG_DEPS "${SWIG_BUILD_DIR}/${item}_pb2.py")
  endforeach (item)

  add_executable(binary_test
      launchers.cpp
      binaries_test.cpp
      )
  target_link_libraries(binary_test
      gtest::main
      shared_model_proto_backend
      integration_framework
      bindings
      )
  target_include_directories(binary_test PUBLIC ${PROJECT_SOURCE_DIR}/test)

  add_dependencies(binary_test irohapy)
  add_test(
      NAME "python_binary_test"
      COMMAND ${CMAKE_COMMAND} -E
      env "PYTHON_INTERPRETER=${PYTHON_EXECUTABLE}" "PYTHONPATH=${SWIG_BUILD_DIR}"
      "ROOT_DIR=${PROJECT_SOURCE_DIR}" $<TARGET_FILE:binary_test> --gtest_filter=*/0*
  )

endif ()
