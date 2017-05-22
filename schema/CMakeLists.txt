message(STATUS "Schema is at: ${IROHA_SCHEMA_DIR}")

function(compile_fbs_to_cpp FBS)
    string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER ${FBS})
    string(REGEX REPLACE "\\.fbs$" ".grpc.fb.cc" GEN_GRPC ${FBS})
    string(REGEX REPLACE "\\.fbs$" ".grpc.fb.h" GEN_GRPC_HEADER ${FBS})
    string(REGEX REPLACE "\\.fbs$" "_h" GEN_TARGET ${FBS})
    add_custom_command(
            OUTPUT ${IROHA_SCHEMA_DIR}/${GEN_HEADER} ${IROHA_SCHEMA_DIR}/${GEN_GRPC} ${IROHA_SCHEMA_DIR}/${GEN_GRPC_HEADER}
            COMMAND "${flatc_EXECUTABLE}" --gen-mutable --cpp --grpc --scoped-enums --no-prefix
            -o "${IROHA_SCHEMA_DIR}" "${IROHA_SCHEMA_DIR}/${FBS}"
            DEPENDS flatc)
    add_custom_target(${GEN_TARGET} DEPENDS "${IROHA_SCHEMA_DIR}/${GEN_HEADER}")
    add_dependencies(flatbuffers ${GEN_TARGET})
endfunction()

compile_fbs_to_cpp(account.fbs)
compile_fbs_to_cpp(asset.fbs)
compile_fbs_to_cpp(actions.fbs)
compile_fbs_to_cpp(endpoint.fbs)
compile_fbs_to_cpp(main.fbs)
compile_fbs_to_cpp(primitives.fbs)
compile_fbs_to_cpp(transaction.fbs)
compile_fbs_to_cpp(query.fbs)

add_library(endpoint_fbs STATIC
    ${IROHA_SCHEMA_DIR}/endpoint.grpc.fb.cc
    )

target_link_libraries(endpoint_fbs
    grpc++
    grpc
    gpr
    pthread
    dl
    flatbuffers
    )
