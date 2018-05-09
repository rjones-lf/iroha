addtest(blob_test blob_test.cpp)
target_link_libraries(blob_test
        shared_model_cryptography_model
        )

addtest(crypto_usage_test crypto_usage_test.cpp)
target_link_libraries(crypto_usage_test
        shared_model_proto_builders
        shared_model_cryptography
        schema
        iroha_amount
        )

addtest(security_signatures_test security_signatures_test.cpp)
target_link_libraries(security_signatures_test
        shared_model_proto_builders
        )
