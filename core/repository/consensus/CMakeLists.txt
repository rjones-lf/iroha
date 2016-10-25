SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_library(event_repository STATIC
    event_repository.cpp
)

add_library(merkle_transaction_repository STATIC
    merkle_transaction_repository.cpp
)