##############################
###     CONFIGURATIONS     ###
##############################
include(FeatureSummary)

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
include(cmake/ccache.cmake)



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

