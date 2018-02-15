#### -DCMAKE_BUILD_TYPE=Coverage

# https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake#L90
SET(FLAGS
    -O1              # first level of optimizations
    -g               # include debug info
    --coverage
    -fprofile-arcs
    -ftest-coverage

    CACHE INTERNAL ""
    )
JOIN("${FLAGS}" " " JOINED_FLAGS)


set(CMAKE_CXX_FLAGS_COVERAGE
    ${JOINED_FLAGS}
    CACHE STRING "Flags used by the C++ compiler during COVERAGE builds."
    FORCE
    )

set(CMAKE_C_FLAGS_COVERAGE
    ${JOINED_FLAGS}
    CACHE STRING "Flags used by the C compiler during COVERAGE builds."
    FORCE
    )

set(CMAKE_EXE_LINKER_FLAGS_COVERAGE
    ""
    CACHE STRING "Flags used for linking binaries during COVERAGE builds."
    FORCE
    )

set(CMAKE_SHARED_LINKER_FLAGS_COVERAGE
    ""
    CACHE STRING "Flags used by the shared libraries linker during COVERAGE builds."
    FORCE
    )

mark_as_advanced(
    CMAKE_CXX_FLAGS_COVERAGE
    CMAKE_C_FLAGS_COVERAGE
    CMAKE_EXE_LINKER_FLAGS_COVERAGE
    CMAKE_SHARED_LINKER_FLAGS_COVERAGE
)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  link_libraries(gcov)
else()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()
