add_library(wsv
    backend/postgresql.cpp
    )

target_link_libraries(wsv
    pqxx)