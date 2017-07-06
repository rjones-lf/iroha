add_library(ametsuchi
    command_executor_stub.cpp
    temporary_wsv_stub.cpp
    mutable_storage_stub.cpp
    ametsuchi_stub.cpp
    )

target_link_libraries(ametsuchi
    rxcpp
    optional
    )