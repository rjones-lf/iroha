add_library(wsv OBJECT
    backend/postgresql.cpp
    )

target_include_directories(wsv PUBLIC ${pqxx_INCLUDE_DIRS})