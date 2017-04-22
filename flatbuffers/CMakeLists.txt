set(FLATBUF_DEST_DIR "${PROJECT_SOURCE_DIR}/include/generated")

file(MAKE_DIRECTORY ${FLATBUF_DEST_DIR})

function(compile_fbs_to_cpp FBS)
  string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER ${FBS})
  string(REGEX REPLACE "\\.fbs$" ".grpc.fb.cc" GEN_GRPC ${FBS})
  string(REGEX REPLACE "\\.fbs$" ".grpc.fb.h" GEN_GRPC_HEADER ${FBS})
  string(REGEX REPLACE "\\.fbs$" "_h" GEN_TARGET ${FBS})
  add_custom_command(
      OUTPUT ${FLATBUF_DEST_DIR}/${GEN_HEADER} ${FLATBUF_DEST_DIR}/${GEN_GRPC} ${FLATBUF_DEST_DIR}/${GEN_GRPC_HEADER}
      COMMAND "${flatc_EXECUTABLE}" --gen-mutable --cpp --grpc
      -o "${FLATBUF_DEST_DIR}"
      "${PROJECT_SOURCE_DIR}/flatbuffers/${FBS}"
      DEPENDS flatc)
  add_custom_target(${GEN_TARGET} DEPENDS "${FLATBUF_DEST_DIR}/${GEN_HEADER}")
  add_dependencies(flatbuffers ${GEN_TARGET})
endfunction()

compile_fbs_to_cpp(account.fbs)
compile_fbs_to_cpp(asset.fbs)
compile_fbs_to_cpp(commands.fbs)
compile_fbs_to_cpp(endpoint.fbs)
compile_fbs_to_cpp(key.fbs)
compile_fbs_to_cpp(main.fbs)
compile_fbs_to_cpp(primitives.fbs)
compile_fbs_to_cpp(transaction.fbs)

add_library(endpoint_fbs STATIC
    ${FLATBUF_DEST_DIR}/endpoint.grpc.fb.cc
    )

target_link_libraries(endpoint_fbs
    grpc++
    grpc++_reflection
    grpc
    grpc_unsecure
    gpr
    pthread
    dl
    flatbuffers
    )