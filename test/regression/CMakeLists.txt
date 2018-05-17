addtest(regression_test regression_test.cpp)

target_link_libraries(regression_test
    application
    integration_framework
    )
