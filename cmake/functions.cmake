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
    target_compile_options(${target} PRIVATE -Wall -Wpedantic)
  elseif ((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR
  (CMAKE_CXX_COMPILER_ID STREQUAL "Intel"))
    target_compile_options(${target} PRIVATE /W3 /WX)
  else ()
    message(AUTHOR_WARNING "Unknown compiler: building target ${target} with default options")
  endif ()
endfunction()

# Creates test "test_name", with "SOURCES" (use string as second argument)
function(addtest test_name SOURCES)
  add_executable(${test_name} ${SOURCES})
  target_link_libraries(${test_name} gtest gmock)
  target_include_directories(${test_name} PUBLIC ${PROJECT_SOURCE_DIR}/test)
  add_test(
      NAME ${test_name}
      COMMAND $<TARGET_FILE:${test_name}>
  )
  strictmode(${test_name})
endfunction()

# Creates benchmark "bench_name", with "SOURCES" (use string as second argument)
function(addbenchmark bench_name SOURCES)
  add_executable(${bench_name} ${SOURCES})
  target_link_libraries(${bench_name} PRIVATE benchmark)
  strictmode(${bench_name})
endfunction()

function(compile_proto_to_cpp PROTORELATIVEPATH)
  # now this function can compile proto in any directory. compiled files will be in the same directory as input file
  get_filename_component(PROTOABSOLUTEPATH ${PROTORELATIVEPATH} REALPATH)
  get_filename_component(PROTODIRPATH      ${PROTOABSOLUTEPATH} DIRECTORY)
  get_filename_component(PROTONAME         ${PROTOABSOLUTEPATH} NAME)
  string(REGEX REPLACE "\\.proto$" ".pb.h" GEN_PB_HEADER ${PROTONAME})
  string(REGEX REPLACE "\\.proto$" ".pb.cc" GEN_PB ${PROTONAME})
  add_custom_command(
      OUTPUT ${PROTODIRPATH}/${GEN_PB_HEADER} ${PROTODIRPATH}/${GEN_PB}
      COMMAND ${CMAKE_COMMAND} -E env LD_LIBRARY_PATH=${protobuf_LIBRARY_DIR}:$ENV{LD_LIBRARY_PATH} "${protoc_EXECUTABLE}"
      ARGS -I${protobuf_INCLUDE_DIR} -I. --cpp_out=${PROTODIRPATH} ${PROTORELATIVEPATH}
      DEPENDS protoc
      WORKING_DIRECTORY ${PROTODIRPATH}
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
