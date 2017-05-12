
ADD_LIBRARY(validator STATIC
    validator.cpp
)

target_link_libraries(validator
    hash
    flatbuffer_service
    logger
)