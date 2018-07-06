
add_library(shared_model_amount_utils
    amount_utils.cpp
    )

target_link_libraries(shared_model_amount_utils
    shared_model_proto_backend
    )
