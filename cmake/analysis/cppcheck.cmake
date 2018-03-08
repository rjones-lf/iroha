# cppcheck performs analysis and saves result in build/cppcheck.xml
# expects to receive array of paths to analyzed directories relative to project root
#
# specify CPPCHECK_BIN variable to set custom path to cppcheck
# variable REPORT_DIR must be specified, otherwise default is used
if(NOT CPPCHECK_BIN)
  find_program(CPPCHECK_BIN cppcheck)
endif()

if(CPPCHECK_BIN)
  message(STATUS "Target cppcheck enabled")
  add_custom_target(cppcheck
      COMMAND ${CPPCHECK_BIN} --xml --xml-version=2 ${CMAKE_SOURCE_DIR}
      -I ${CMAKE_SOURCE_DIR}/irohad/        # include (include_directories(...)
      -I ${CMAKE_SOURCE_DIR}/iroha-cli/     # include
      -I ${CMAKE_SOURCE_DIR}/test/          # include
      -I ${CMAKE_SOURCE_DIR}/libs/          # include
      -I ${CMAKE_SOURCE_DIR}/shared_model/  # include
      -i ${CMAKE_SOURCE_DIR}/external/      # exclude
      -i ${CMAKE_SOURCE_DIR}/schema/        # exclude
      -i ${CMAKE_BINARY_DIR}                # exclude
      --enable=all
      2> ${REPORT_DIR}/cppcheck.xml
      COMMENT "Analyzing sources with cppcheck" VERBATIM
      )
endif()
