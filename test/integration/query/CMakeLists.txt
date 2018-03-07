addtest(query_acceptance_test
        query_acceptance_test.cpp
        )
target_link_libraries(query_acceptance_test
        application
        gtest
        integration_framework
        )
