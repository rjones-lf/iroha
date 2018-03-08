# starting from CMake 3.4 it should not be before PROJECT() command
# https://crascit.com/2016/04/09/using-ccache-with-cmake/
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    if(CMAKE_GENERATOR STREQUAL "Xcode")
        # Set up ccache for xcode

        # https://gitlab.kitware.com/henryiii/cmake/blob/cache/Modules/UseCompilerCache.cmake
        file(WRITE "${CMAKE_BINARY_DIR}/launch-c" ""
            "#!/bin/sh\n"
            "\n"
            "# Xcode generator doesn't include the compiler as the\n"
            "# first argument, Ninja and Makefiles do. Handle both cases.\n"
            "if [[ \"$1\" = \"${CMAKE_C_COMPILER}\" ]] ; then\n"
            "    shift\n"
            "fi\n"
            "\n"
            "export CCACHE_CPP2=true\n"
            "exec \"${CCACHE_PROGRAM}\" \"${CMAKE_C_COMPILER}\" \"$@\"\n"
            )

        file(WRITE "${CMAKE_BINARY_DIR}/launch-cxx" ""
            "#!/bin/sh\n"
            "\n"
            "# Xcode generator doesn't include the compiler as the\n"
            "# first argument, Ninja and Makefiles do. Handle both cases.\n"
            "if [[ \"$1\" = \"${CMAKE_CXX_COMPILER}\" ]] ; then\n"
            "    shift\n"
            "fi\n"
            "\n"
            "export CCACHE_CPP2=true\n"
            "exec \"${CCACHE_PROGRAM}\" \"${CMAKE_CXX_COMPILER}\" \"$@\"\n"
            )

        execute_process(COMMAND chmod a+rx
            "${CMAKE_BINARY_DIR}/launch-c"
            "${CMAKE_BINARY_DIR}/launch-cxx"
            )

        # Set Xcode project attributes to route compilation and linking
        # through our scripts
        set(CMAKE_XCODE_ATTRIBUTE_CC         "${CMAKE_BINARY_DIR}/launch-c")
        set(CMAKE_XCODE_ATTRIBUTE_CXX        "${CMAKE_BINARY_DIR}/launch-cxx")
        set(CMAKE_XCODE_ATTRIBUTE_LD         "${CMAKE_BINARY_DIR}/launch-c")
        set(CMAKE_XCODE_ATTRIBUTE_LDPLUSPLUS "${CMAKE_BINARY_DIR}/launch-cxx")
    else()
        # Support Unix Makefiles and Ninja
        set(CMAKE_C_COMPILER_LAUNCHER   "${CCACHE_PROGRAM}")
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    endif()
endif()