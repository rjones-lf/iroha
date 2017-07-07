add_library(networking
    impl/network_api.cpp
    peer_communication_stub.cpp
    )

target_link_libraries(networking PUBLIC
    rxcpp
    model
    ametsuchi
    )
