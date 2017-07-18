add_executable(test_network test_network.cpp)
target_link_libraries(test_network
    uvw
    spdlog
    crypto
    peer
    peer_state
    )
