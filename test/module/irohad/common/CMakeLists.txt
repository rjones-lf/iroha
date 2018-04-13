AddTest(blob_converter_test blob_converter_test.cpp)
target_link_libraries(blob_converter_test
    common
    )

AddTest(block_insertion_test raw_block_loader_test.cpp)
target_link_libraries(block_insertion_test
    json_model_converters
    raw_block_insertion
    shared_model_cryptography_model
    logger
    )
