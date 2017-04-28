
add_library(datetime STATIC datetime.cpp)
add_library(logger STATIC logger.cpp)

target_link_libraries(logger
    datetime
    spdlog
)

add_library(random STATIC random.cpp)

add_library(exception STATIC
    exception.cpp
)

add_library(expected STATIC
  expected.cpp
)
target_link_libraries(expected
    exception
    flatbuffers
)

add_library(timer STATIC timer.cpp)

add_library(ip_tools STATIC ip_tools.cpp)
target_link_libraries(ip_tools
    logger
)
