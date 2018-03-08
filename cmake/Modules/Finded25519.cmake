add_library(ed25519 UNKNOWN IMPORTED)

find_path(ed25519_INCLUDE_DIR ed25519/ed25519.h)
mark_as_advanced(ed25519_INCLUDE_DIR)

find_library(ed25519_LIBRARY ed25519)
mark_as_advanced(ed25519_LIBRARY)

find_package_handle_standard_args(ed25519 DEFAULT_MSG
    ed25519_INCLUDE_DIR
    ed25519_LIBRARY
    )

set(URL https://github.com/hyperledger/iroha-ed25519)
set(VERSION e7188b8393dbe5ac54378610d53630bd4a180038)
set_target_description(ed25519 "Digital signature algorithm" ${URL} ${VERSION})

iroha_get_lib_name(EDLIB ed25519 SHARED)
set(EDLIB_PATH ${CMAKE_BINARY_DIR}/${EDLIB})

if (NOT ed25519_FOUND)
  externalproject_add(hyperledger_ed25519
      GIT_REPOSITORY ${URL}
      GIT_TAG        ${VERSION}
      CMAKE_ARGS
          -DTESTING=OFF
          -DBUILD=SHARED  # find_package searches

          -G${CMAKE_GENERATOR}
          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
          -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}
          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
          -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
          -DCMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}
          -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}
          -DCMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_BINARY_DIR}
      BUILD_BYPRODUCTS
          ${EDLIB_PATH}
      INSTALL_COMMAND "" # remove install step
      TEST_COMMAND    "" # remove test step
      UPDATE_COMMAND  "" # remove update step
      )
  externalproject_get_property(hyperledger_ed25519 binary_dir)
  externalproject_get_property(hyperledger_ed25519 source_dir)
  set(ed25519_INCLUDE_DIR ${source_dir}/include)
  set(ed25519_LIBRARY ${EDLIB_PATH})
  file(MAKE_DIRECTORY ${ed25519_INCLUDE_DIR})
  link_directories(${binary_dir})

  add_dependencies(ed25519 hyperledger_ed25519)
endif ()

set_target_properties(ed25519 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${ed25519_INCLUDE_DIR}
    IMPORTED_LOCATION ${ed25519_LIBRARY}
    )

if(ENABLE_LIBS_PACKAGING)
  add_install_step_for_lib(${ed25519_LIBRARY})
endif()
