# here will be the code, which is responsible for connection to ametsuchi
message(STATUS "Ametsuchi")

add_subdirectory(block_store)
add_subdirectory(index)
add_subdirectory(wsv)

add_library(storage
    impl/ametsuchi.cpp
    index_mediator.cpp
    $<TARGET_OBJECTS:block_store>
    $<TARGET_OBJECTS:index>
    $<TARGET_OBJECTS:wsv>
    )

target_link_libraries(storage PUBLIC
    schema
    cpp_redis
    pqxx
    )