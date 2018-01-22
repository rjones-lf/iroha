add_library(rttr UNKNOWN IMPORTED)

find_path(rttr_INCLUDE_DIR rttr/registration)
mark_as_advanced(rttr_INCLUDE_DIR)

find_library(rttr_LIBRARY rttr_core)
mark_as_advanced(rttr_LIBRARY)

find_package_handle_standard_args(rttr DEFAULT_MSG
        rttr_INCLUDE_DIR
        rttr_LIBRARY
        )


set(URL https://github.com/warchant/rttr)
set(VERSION a7c2fb3d14996c4a8d8383917864780e47d5c618) # master
set_target_description(rttr "C++ Reflection Library" ${URL} ${VERSION})


if (NOT rttr_FOUND)
    ExternalProject_Add(warchant_rttr
            GIT_REPOSITORY ${URL}
            GIT_TAG        ${VERSION}
            CMAKE_ARGS -DCMAKE_BUILD_TYPE=Debug
                -DBUILD_UNIT_TESTS=OFF
                -DBUILD_BENCHMARKS=OFF
                -DBUILD_EXAMPLES=OFF
                -DBUILD_DOCUMENTATION=OFF
                -DBUILD_STATIC=OFF
                -DUSE_PCH=OFF
                # remove postfix _d for debug builds
                -DCMAKE_DEBUG_POSTFIX=
            INSTALL_COMMAND   "" # remove install step
            TEST_COMMAND      "" # remove test step
            UPDATE_COMMAND    "" # remove update step
            )
    ExternalProject_Get_Property(warchant_rttr source_dir binary_dir)
    set(rttr_INCLUDE_DIR ${source_dir}/src)
    set(rttr_LIBRARY   ${binary_dir}/src/rttr/${CMAKE_SHARED_LIBRARY_PREFIX}rttr_core${CMAKE_SHARED_LIBRARY_SUFFIX})
    file(MAKE_DIRECTORY ${rttr_INCLUDE_DIR})
    add_dependencies(rttr warchant_rttr)
endif ()

set_target_properties(rttr PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${rttr_INCLUDE_DIR}
        IMPORTED_LOCATION ${rttr_LIBRARY}
        )
