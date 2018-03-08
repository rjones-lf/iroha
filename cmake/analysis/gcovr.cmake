# https://github.com/bilke/cmake-modules/blob/master/CodeCoverage.cmake

if(NOT CMAKE_BUILD_TYPE STREQUAL "Coverage")
  message(WARNING "Current build type is ${CMAKE_BUILD_TYPE}, but Coverage is recommended to collect better coverage")
endif()

# gcovr performs coverage analysis and saves result in build/gcovr.xml
# expects to receive array of paths to analyzed directories relative to project root
#
# specify GCOVR_BIN variable to set custom path to gcovr
# variable REPORT_DIR must be specified, otherwise default is used
if(NOT GCOVR_BIN)
  find_program(GCOVR_BIN gcovr)
endif()

if(GCOVR_BIN)
  message(STATUS "Target gcovr enabled")

  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(GCOV_BACKEND "llvm-cov gcov")
  else()
    set(GCOV_BACKEND "gcov")
  endif()

  add_custom_target(gcovr
      COMMAND ${GCOVR_BIN} -s -x -r '${CMAKE_SOURCE_DIR}'
      -e '${CMAKE_SOURCE_DIR}/external/*'
      -e '${CMAKE_SOURCE_DIR}/schema/*'
      -e '${CMAKE_BINARY_DIR}/*'
      --gcov-executable='${GCOV_BACKEND}'
      -o ${REPORT_DIR}/gcovr.xml
      COMMENT "Collecting coverage data with gcovr"
      )
endif()
