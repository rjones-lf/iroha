set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/test_bin)

include(../../../../cmake/functions.cmake)
include_directories(../include)

add_library(tx_generator generator/tx_generator.h)
set_target_properties(tx_generator PROPERTIES LINKER_LANGUAGE CXX)

AddTest(tx_generator_test generator/tx_generator_test.cc)
target_link_libraries(tx_generator_test PRIVATE tx_generator flatbuffers)

AddTest(module_lmdb module/lmdb_test.cc)
target_link_libraries(module_lmdb PRIVATE LMDB spdlog ${LIBAMETSUCHI_NAME})

AddTest(currency_test ametsuchi/currency_test.cc)
target_link_libraries(currency_test PRIVATE ${LIBAMETSUCHI_NAME})

AddTest(ametsuchi_test ametsuchi/ametsuchi.cc)
target_link_libraries(ametsuchi_test PRIVATE ${LIBAMETSUCHI_NAME} tx_generator)

AddTest(merkle_test ametsuchi/merkle_test.cc)
target_link_libraries(merkle_test PRIVATE ${LIBAMETSUCHI_NAME})

AddTest(circular_stack_iter_test ametsuchi/circular_stack_iter_test.cc)
target_link_libraries(circular_stack_iter_test PRIVATE ${LIBAMETSUCHI_NAME})

AddTest(circular_stack_test ametsuchi/circular_stack_test.cc)
target_link_libraries(circular_stack_test PRIVATE ${LIBAMETSUCHI_NAME})
