# Compile the specified target as a modern, strict C++.
function(strictmode target)
  # Require pure C++14 standard.
  set_target_properties(${target} PROPERTIES
      CXX_STANDARD 14
      CXX_STANDARD_REQUIRED ON
      CXX_EXTENSIONS OFF
      )
  # Enable more warnings and turn them into compile errors.
  if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
  (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR
  (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
    target_compile_options(${target} PRIVATE -Wall -Wpedantic -Werror)
  elseif ((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR
  (CMAKE_CXX_COMPILER_ID STREQUAL "Intel"))
    target_compile_options(${target} PRIVATE /W3 /WX)
  else ()
    message(AUTHOR_WARNING "Unknown compiler: building target ${target} with default options")
  endif ()
endfunction()

# Creates test "test_name"
function(addtest test_name)
  if (COVERAGE)
    set(test_xml_output --gtest_output=xml:${REPORT_DIR}/xunit-${test_name}.xml)
  endif ()
  add_executable(${test_name} ${ARGN})
  target_link_libraries(${test_name}
      GTest::main
      GMock::gmock
      )
  target_include_directories(${test_name} PUBLIC ${PROJECT_SOURCE_DIR}/test)
  add_test(
      NAME ${test_name}
      COMMAND $<TARGET_FILE:${test_name}> ${test_xml_output}
  )
  strictmode(${test_name})
  if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR
  (CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR
  (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
    target_compile_options(${test_name} PRIVATE -Wno-inconsistent-missing-override)
  else ()
    message(AUTHOR_WARNING "Unknown compiler: building target ${target} with default options")
  endif ()
endfunction()

# Creates benchmark "bench_name", with "SOURCES" (use string as second argument)
function(addbenchmark bench_name SOURCES)
  add_executable(${bench_name} ${SOURCES})
  target_link_libraries(${bench_name} PRIVATE benchmark)
  strictmode(${bench_name})
endfunction()

function(compile_proto_to_cpp PROTO)
  string(REGEX REPLACE "\\.proto$" ".pb.h" GEN_PB_HEADER ${PROTO})
  string(REGEX REPLACE "\\.proto$" ".pb.cc" GEN_PB ${PROTO})
  add_custom_command(
      OUTPUT ${IROHA_SCHEMA_DIR}/${GEN_PB_HEADER} ${IROHA_SCHEMA_DIR}/${GEN_PB}
      COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=${protobuf_LIBRARY_DIR}:$ENV{LD_LIBRARY_PATH} "${protoc_EXECUTABLE}"
      ARGS -I${protobuf_INCLUDE_DIR} -I. --cpp_out=${IROHA_SCHEMA_DIR} ${PROTO}
      DEPENDS protoc
      WORKING_DIRECTORY ${IROHA_SCHEMA_DIR}
      )
endfunction()


function(compile_proto_to_grpc_cpp PROTO)
  compile_proto_to_cpp(${PROTO})
  string(REGEX REPLACE "\\.proto$" ".grpc.pb.h" GEN_GRPC_PB_HEADER ${PROTO})
  string(REGEX REPLACE "\\.proto$" ".grpc.pb.cc" GEN_GRPC_PB ${PROTO})
  add_custom_command(
      OUTPUT ${IROHA_SCHEMA_DIR}/${GEN_GRPC_PB_HEADER} ${IROHA_SCHEMA_DIR}/${GEN_GRPC_PB}
      COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=${protobuf_LIBRARY_DIR}:$ENV{LD_LIBRARY_PATH} "${protoc_EXECUTABLE}"
      ARGS -I${protobuf_INCLUDE_DIR} -I. --grpc_out=${IROHA_SCHEMA_DIR} --plugin=protoc-gen-grpc="${grpc_CPP_PLUGIN}" ${PROTO}
      DEPENDS grpc_cpp_plugin
      WORKING_DIRECTORY ${IROHA_SCHEMA_DIR}
      )
endfunction()


function(compile_proto_to_python PROTO)
  string(REGEX REPLACE "\\.proto$" "_pb2.py" PY_PB ${PROTO})
  add_custom_command(
      OUTPUT ${SWIG_BUILD_DIR}/${PY_PB}
      COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=${protobuf_LIBRARY_DIR}:$ENV{LD_LIBRARY_PATH} "${protoc_EXECUTABLE}"
      ARGS -I${protobuf_INCLUDE_DIR} -I. --python_out=${SWIG_BUILD_DIR} ${PROTO}
      DEPENDS protoc
      WORKING_DIRECTORY ${IROHA_SCHEMA_DIR}
      )
endfunction()


macro(set_target_description target description url commit)
  set_package_properties(${target}
      PROPERTIES
      URL ${url}
      DESCRIPTION ${description}
      PURPOSE "commit: ${commit}"
      )
endmacro()


macro(add_install_step_for_bin target)
  install(TARGETS ${target}
      RUNTIME DESTINATION bin
      CONFIGURATIONS Release
      COMPONENT binaries)
endmacro()


macro(add_install_step_for_lib libpath)
  # full path with resolved symlinks:
  # /usr/local/lib/libprotobuf.so -> /usr/local/lib/libprotobuf.so.13.0.0
  get_filename_component(lib_major_minor_patch ${libpath} REALPATH)

  install(FILES ${lib_major_minor_patch}
      DESTINATION lib
      CONFIGURATIONS Release
      COMPONENT libraries)
endmacro()


macro(remove_line_terminators str output)
  string(REGEX REPLACE "\r|\n" "" ${output} ${str})
endmacro()


macro(get_git_revision commit)
  find_package(Git)
  execute_process(
      COMMAND ${GIT_EXECUTABLE} rev-parse HEAD
      OUTPUT_VARIABLE ${commit}
      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  )
endmacro()


function(iroha_get_lib_name out lib type)
  if(type STREQUAL "STATIC")
    set(${out} ${CMAKE_STATIC_LIBRARY_PREFIX}${lib}${CMAKE_STATIC_LIBRARY_SUFFIX} PARENT_SCOPE)
  elseif(type STREQUAL "SHARED")
    set(${out} ${CMAKE_SHARED_LIBRARY_PREFIX}${lib}${CMAKE_SHARED_LIBRARY_SUFFIX} PARENT_SCOPE)
  else()
    message(FATAL_ERROR "type can be either STATIC or SHARED")
  endif()
endfunction()


function(JOIN VALUES GLUE OUTPUT)
  string (REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()
