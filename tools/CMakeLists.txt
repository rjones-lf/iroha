SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

###########################
#      make sumeragi      #
###########################
add_executable(make_sumeragi make_sumeragi.cpp)
target_link_libraries(make_sumeragi
    base64
    signature
    json
)

###########################
#    issue transaction    #
###########################

# automatic generation source files by using templates
execute_process(COMMAND bash "-c" "cd $IROHA_HOME/tools/helper; ./allbuild.sh")

add_executable(issue_transaction
    issue_transaction.cpp
)

target_link_libraries(issue_transaction
    config_manager
    peer_service
    logger
    transaction_builder
)

###########################
#      send izanami       #
###########################

add_executable(send_izanami_test
    send_izanami.cpp
)

target_link_libraries(send_izanami_test
    peer_service
)