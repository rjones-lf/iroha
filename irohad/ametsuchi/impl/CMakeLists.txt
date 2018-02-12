add_library(block_storage_nudb
        block_storage_nudb.cpp
        )
target_link_libraries(block_storage_nudb
        nudb
        boost
        logger
        )
