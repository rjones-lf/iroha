add_library(flat_file
        flat_file.cpp
        )
target_link_libraries(flat_file
        nudb
        logger
        )
