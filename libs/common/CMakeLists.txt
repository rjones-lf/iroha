add_library(libs_common
    files.cpp
    result.hpp
    )

target_link_libraries(libs_common
    logger
    )

add_library(common INTERFACE)
target_include_directories(common INTERFACE
    ${PROJECT_SOURCE_DIR}/libs/common
    )
target_link_libraries(common INTERFACE
    optional
    )
