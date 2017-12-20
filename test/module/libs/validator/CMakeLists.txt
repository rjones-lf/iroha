addtest(validator_test domain_name_validator_test.cpp)
target_link_libraries(validator_test
    validator
    )

addtest(address_validator_test address_validator_test.cpp)
target_link_libraries(address_validator_test
        validator
        )
