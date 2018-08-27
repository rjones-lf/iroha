add_library(synchronizer
    impl/synchronizer_impl.cpp
    )

target_link_libraries(synchronizer
    common
    rxcpp
    logger
    )
