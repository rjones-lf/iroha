SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

SET(JSON_PATH "${PROJECT_SOURCE_DIR}/core/vendor/json")

# Datetime
ADD_LIBRARY(datetime  STATIC datetime.cpp)

# Logger
ADD_LIBRARY(logger  STATIC logger.cpp)
target_link_libraries(logger
  datetime
)

# Random
ADD_LIBRARY(random  STATIC random.cpp)


# Exception
ADD_LIBRARY(exception  STATIC exception.cpp)

# Terminate
ADD_LIBRARY(terminate  STATIC terminate.cpp)
