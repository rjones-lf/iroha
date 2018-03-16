add_library(logger STATIC logger.cpp)
target_link_libraries(logger
    spdlog
)
