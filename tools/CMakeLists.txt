
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(JSON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/json/src")
SET(ED25519_PATH "${PROJECT_SOURCE_DIR}/core/vendor/ed25519")

link_directories(
    ${ED25519_PATH}/lib
)

include_directories(
    ${JSON_PATH}
)

# make config
add_executable(make_sumeragi make_sumeragi )
target_link_libraries(make_sumeragi
    base64
    signature
)
