add_library(iroha_amount
        amount.cpp
        )

target_link_libraries(
        iroha_amount
        optional
)