##############################
###     CONFIGURATIONS     ###
##############################
include(FeatureSummary)

# cmake policies
include(cmake/policies.cmake)

# user functions
include(cmake/functions.cmake)

# flags and config options
include(cmake/config/all.cmake)
include(cmake/config/release.cmake)
include(cmake/config/debug.cmake)
include(cmake/config/coverage.cmake)

# all 3rd-party dependencies
# should be last in this section
include(cmake/dependencies.cmake)



##############################
###        CCACHE          ###
##############################
# starting from CMake 3.4 it should not be before PROJECT() command
# https://crascit.com/2016/04/09/using-ccache-with-cmake/
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  # Set up wrapper scripts
  set(C_LAUNCHER   "${CCACHE_PROGRAM}")
  set(CXX_LAUNCHER "${CCACHE_PROGRAM}")
  configure_file(launch-c.in   launch-c)
  configure_file(launch-cxx.in launch-cxx)
  execute_process(COMMAND chmod a+rx
      "${CMAKE_BINARY_DIR}/launch-c"
      "${CMAKE_BINARY_DIR}/launch-cxx"
      )

  if(CMAKE_GENERATOR STREQUAL "Xcode")
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



##############################
###        ANALYSIS        ###
##############################
set(REPORT_DIR ${CMAKE_BINARY_DIR}/reports CACHE STRING "Analysis report dir")
file(MAKE_DIRECTORY ${REPORT_DIR})

if(COVERAGE)
  include(cmake/analysis/gcovr.cmake)
endif()
# 1. make attempt to find analysis tool
# 2. if successful, enable according target. if not - does nothing.
include(cmake/analysis/cppcheck.cmake)
include(cmake/analysis/clang-format.cmake)

