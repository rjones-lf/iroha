add_library(index OBJECT
    backend/redis.cpp
    )

target_include_directories(index PUBLIC
    ${CPP_REDIS_INCLUDE_DIRS}
    ${optional_INCLUDE_DIRS}
    )