add_library(rxcpp INTERFACE IMPORTED)

# TODO 2019-04-02 lebdron: IR-346 Update Rx commit

find_package_handle_standard_args(rxcpp DEFAULT_MSG
    rxcpp_INCLUDE_DIR
    )

set(URL https://github.com/Reactive-Extensions/rxcpp.git)
set(VERSION 7c79c18ab252eb37bcf122c75070f8d557868c08)
set_target_description(rxcpp "Library for reactive programming" ${URL} ${VERSION})

externalproject_add(reactive_extensions_rxcpp
    GIT_REPOSITORY ${URL}
    GIT_TAG        ${VERSION}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    PATCH_COMMAND ${GIT_EXECUTABLE} apply ${PROJECT_SOURCE_DIR}/patch/rxcpp-synchronize-lock-recursion-thread-safety.patch
    INSTALL_COMMAND "" # remove install step
    TEST_COMMAND "" # remove test step
    )
externalproject_get_property(reactive_extensions_rxcpp source_dir)
set(rxcpp_INCLUDE_DIR ${source_dir}/Rx/v2/src)
file(MAKE_DIRECTORY ${rxcpp_INCLUDE_DIR})

add_dependencies(rxcpp reactive_extensions_rxcpp)

set_target_properties(rxcpp PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${rxcpp_INCLUDE_DIR}
    )
