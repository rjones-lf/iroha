ADD_LIBRARY(runtime STATIC
    runtime.cpp
    validator.cpp
)

target_link_libraries(runtime
    repository
)