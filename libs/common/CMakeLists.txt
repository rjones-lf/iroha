add_library(libs_common
    files.cpp
    )

target_link_libraries(libs_common
    logger
    )

add_library(common INTERFACE)
target_include_directories(common INTERFACE
    ${PROJECT_SOURCE_DIR}/libs/common
    )

