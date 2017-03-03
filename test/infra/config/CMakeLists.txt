add_executable(ensure_sumeragi_json_format_test
    ensure_sumeragi_json_format.cpp
)

target_link_libraries(ensure_sumeragi_json_format_test
    config_manager
    json
    gtest
)

add_test(
    NAME ensure_sumeragi_json_format_test
    COMMAND $<TARGET_FILE:ensure_sumeragi_json_format_test>
)
