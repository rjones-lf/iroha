function(JOIN VALUES GLUE OUTPUT)
  string (REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
endfunction()

macro(mkdir arg)
  file(MAKE_DIRECTORY ${arg})
endmacro()

# find all .{h,hpp,c,cc,cpp} files in given directory
# append result to last two arguments: headers, sources
# usage: find_all_sources(${CMAKE_SOURCE_DIR}/irohad HEADERS SOURCES)
# now you have HEADERS and SOURCES vars
macro(find_all_sources root headers sources)
  file(GLOB_RECURSE CPPS ${root}/*.cpp)
  file(GLOB_RECURSE CS ${root}/*.c)
  file(GLOB_RECURSE CCS ${root}/*.cc)

  file(GLOB_RECURSE HS ${root}/*.h)
  file(GLOB_RECURSE HPPS ${root}/*.hpp)

  list(APPEND ${headers} ${HS} ${HPPS})
  list(APPEND ${sources} ${CS} ${CPPS} ${CCS})
endmacro()



# cppcheck performs analysis and saves result in build/cppcheck*.xml
# expects to receive array of paths to analyzed directories relative to project root
#
# specify CPPCHECK_BIN variable to set custom path to cppcheck
# variable REPORT_DIR must be specified, otherwise default is used
macro(scan_cppcheck)
  if(NOT CPPCHECK_BIN)
    find_program(CPPCHECK_BIN cppcheck
        HINTS /usr/bin /usr/local/bin
        )
  endif()

  if(NOT REPORT_DIR)
    set(REPORT_DIR "${CMAKE_BINARY_DIR}")
    message(STATUS "REPORT_DIR default is ${REPORT_DIR}")
  endif()

  if(CPPCHECK_BIN-NOTFOUND)
    message(WARNING "cppcheck can not be found in PATH. Target cppcheck is not available.")
  else()
    foreach(arg ${ARGV})
      find_all_sources(${CMAKE_SOURCE_DIR}/${arg} HDRS SRCS)
    endforeach()

    message(STATUS "Target cppcheck enabled")
    add_custom_target(cppcheck
      COMMAND ${CPPCHECK_BIN} --xml --xml-version=2 ${SRCS} -I${HDRS} -j4 --enable=all 2>${REPORT_DIR}/cppcheck.xml 1>/dev/null
      COMMENT "Analyzing sources with cppcheck" VERBATIM
      )

    # message(STATUS "executing cppcheck at ${CPPCHECK_BIN}")
    # execute_process(
    #     COMMAND ${CPPCHECK_BIN} --xml --xml-version=2 ${SRCS} -I${HDRS} -j4 --enable=all
    #     ERROR_VARIABLE ERROR
    #     )
    ## save execution report
    # file(WRITE ${REPORT_DIR}/cppcheck.xml ${ERROR})
  endif()
endmacro()




# gcovr performs coverage analysis and saves result in build/gcovr*.xml
# expects to receive array of paths to analyzed directories relative to project root
#
# specify GCOVR_BIN variable to set custom path to gcovr
# variable REPORT_DIR must be specified, otherwise default is used
macro(scan_gcovr)
  if(NOT GCOVR_BIN)
    find_program(GCOVR_BIN gcovr
        HINTS /usr/bin /usr/local/bin
        )
  endif()

  if(NOT REPORT_DIR)
    set(REPORT_DIR "${CMAKE_BINARY_DIR}")
    message(STATUS "REPORT_DIR default is ${REPORT_DIR}")
  endif()

  if(GCOVR_BIN-NOTFOUND)
    message(WARNING "gcovr can not be found in PATH. Target gcovr is not available.")
  else()
    # join all input files
    foreach(arg ${ARGV})
      find_all_sources(${CMAKE_SOURCE_DIR}/${arg} HDRS SRCS)
    endforeach()

    list(APPEND ${SRCS} HDRS)
    message(STATUS "Target gcovr enabled")
    add_custom_target(gcovr
      COMMAND ${GCOVR_BIN} -x -r ${CMAKE_SOURCE_DIR} > ${REPORT_DIR}/gcovr.xml
      COMMENT "Collecting coverage data with gcovr"
      )

    # message(STATUS "executing gcovr at ${GCOVR_BIN}")
    # execute_process(
    #     COMMAND ${GCOVR_BIN} -x -r ${CMAKE_SOURCE_DIR} 
    #     OUTPUT_VARIABLE OUTPUT
    # )
    # file(WRITE ${REPORT_DIR}/gcovr.xml OUTPUT)
  endif()
endmacro()
