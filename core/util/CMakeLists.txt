add_library(datetime STATIC datetime.cpp)
add_library(logger STATIC logger.cpp)

target_link_libraries(logger
  datetime
)

add_library(convert_string  STATIC convert_string.cpp)
add_library(random          STATIC random.cpp)
add_library(exception       STATIC exception.cpp)
add_library(terminate       STATIC terminate.cpp)
