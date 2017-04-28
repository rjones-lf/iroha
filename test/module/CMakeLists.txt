set(CMAKE_BUILD_TYPE Debug)

SET(EXECUTABLE_OUTPUT_P
ATH ${PROJECT_BINARY_DIR}/test_bin)

add_subdirectory(config)
add_subdirectory(crypto)
add_subdirectory(expected)
add_subdirectory(membership_service)
#add_subdirectory(infra/repository)
#add_subdirectory(infra/service)
#add_subdirectory(infra/config)
# commented because these tests do nothing
# TODO: make them do something :)
# add_subdirectory(consensus)
#add_subdirectory(vendor)
#add_subdirectory(validation)
#add_subdirectory(connection)
#add_subdirectory(transaction_builder)

add_subdirectory(service)
