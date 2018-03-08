add_library(gflags UNKNOWN IMPORTED)

find_path(gflags_INCLUDE_DIR gflags/gflags.h)
mark_as_advanced(gflags_INCLUDE_DIR)

find_library(gflags_LIBRARY gflags)
mark_as_advanced(gflags_LIBRARY)

find_package_handle_standard_args(gflags DEFAULT_MSG
    gflags_INCLUDE_DIR
    gflags_LIBRARY
    )

set(URL https://github.com/gflags/gflags.git)
set(VERSION f8a0efe03aa69b3336d8e228b37d4ccb17324b88)
set_target_description(gflags "Flag parsing engine" ${URL} ${VERSION})

iroha_get_lib_name(GFLAGSLIB gflags STATIC)

if (NOT gflags_FOUND)
  externalproject_add(gflags_gflags
      GIT_REPOSITORY   ${URL}
      GIT_TAG          ${VERSION}
      CMAKE_ARGS
          -G${CMAKE_GENERATOR}
          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
          -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}
          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
          -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}
          -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}
          -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}
      BUILD_BYPRODUCTS
          ${CMAKE_BINARY_DIR}${XCODE_EXT}/${GFLAGSLIB}
      INSTALL_COMMAND  "" # remove install step
      TEST_COMMAND     "" # remove test step
      UPDATE_COMMAND   "" # remove update step
      )
  externalproject_get_property(gflags_gflags binary_dir)
  set(gflags_INCLUDE_DIR ${binary_dir}/include)

  set(gflags_LIBRARY ${CMAKE_BINARY_DIR}${XCODE_EXT}/${GFLAGSLIB})

  file(MAKE_DIRECTORY ${gflags_INCLUDE_DIR})

  add_dependencies(gflags gflags_gflags)
endif ()

set_target_properties(gflags PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${gflags_INCLUDE_DIR}
    IMPORTED_LOCATION ${gflags_LIBRARY}
    INTERFACE_LINK_LIBRARIES "pthread"
    )
