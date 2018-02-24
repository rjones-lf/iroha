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

iroha_get_lib_name(EDLIB ed25519 STATIC)

if (NOT ed25519_FOUND)
  externalproject_add(hyperledger_ed25519
      GIT_REPOSITORY ${URL}
      GIT_TAG        ${VERSION}
      CMAKE_ARGS
          -DEDIMPL=ref10
          -DHASH=sha3_brainhub
          -DRANDOM=dev_urandom

          -DTESTING=OFF
          -DBUILD_TESTING=OFF
          -DBUILD=STATIC

          -G${CMAKE_GENERATOR}
          -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
          -DCMAKE_C_COMPILER_LAUNCHER=${CMAKE_C_COMPILER_LAUNCHER}
          -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
          -DCMAKE_CXX_COMPILER_LAUNCHER=${CMAKE_CXX_COMPILER_LAUNCHER}
      BUILD_BYPRODUCTS
          ${EP_PREFIX}/src/hyperledger_ed25519-build${XCODE_EXT}/${EDLIB}
      INSTALL_COMMAND "" # remove install step
      TEST_COMMAND    "" # remove test step
      UPDATE_COMMAND  "" # remove update step
      )
  externalproject_get_property(hyperledger_ed25519 binary_dir)
  externalproject_get_property(hyperledger_ed25519 source_dir)
  set(ed25519_INCLUDE_DIR ${source_dir}/include)
  set(ed25519_LIBRARY     ${binary_dir}${XCODE_EXT}/${EDLIB})
  file(MAKE_DIRECTORY ${ed25519_INCLUDE_DIR})

  add_dependencies(ed25519 hyperledger_ed25519)
endif ()

set_target_properties(ed25519 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${ed25519_INCLUDE_DIR}
    IMPORTED_LOCATION ${ed25519_LIBRARY}
    )
