# COMPILE_PROTO(S1 H1 ${PROTO_DIR}/api.proto)

add_library(generator STATIC
    generator.cc generator.h
    ${S1} ${H1} # compiled api.pb.{cc,h}
    tx_generator.h
    )

target_link_libraries(generator PUBLIC
    ${PROTOBUF_LIBRARIES}
    crypto
    )