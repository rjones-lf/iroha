add_library(ed25519 UNKNOWN IMPORTED)

find_path(ed25519_INCLUDE_DIR ed25519/ed25519.h)
mark_as_advanced(ed25519_INCLUDE_DIR)

find_library(ed25519_LIBRARY ed25519)
mark_as_advanced(ed25519_LIBRARY)

find_package_handle_standard_args(ed25519 DEFAULT_MSG
    ed25519_INCLUDE_DIR
    ed25519_LIBRARY
    )

set(URL https://github.com/warchant/ed25519.git)
set(VERSION 0d31b082e0f0d4144fe598fb0f2a120e36de2a8f)
set_target_description(ed25519 "Digital signature algorithm" ${URL} ${VERSION})

if (NOT ed25519_FOUND)
  externalproject_add(warchant_ed25519
      GIT_REPOSITORY ${URL}
      GIT_TAG        ${VERSION}
      CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DEDIMPL=ref10
        -DHASH=sha3_brainhub
        -DRANDOM=dev_urandom
        -DBUILD=SHARED
        -DBENCHMARKING=OFF
        -DCOVERAGE=OFF
        -DTESTING=OFF
      INSTALL_COMMAND "" # remove install step
      TEST_COMMAND    "" # remove test step
      UPDATE_COMMAND  "" # remove update step
      )
  externalproject_get_property(warchant_ed25519 binary_dir)
  externalproject_get_property(warchant_ed25519 source_dir)
  set(ed25519_INCLUDE_DIR ${source_dir}/include)
  set(ed25519_LIBRARY ${binary_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}ed25519${CMAKE_SHARED_LIBRARY_SUFFIX})
  file(MAKE_DIRECTORY ${ed25519_INCLUDE_DIR})

  add_dependencies(ed25519 warchant_ed25519)
endif ()

set_target_properties(ed25519 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${ed25519_INCLUDE_DIR}
    IMPORTED_LOCATION ${ed25519_LIBRARY}
    )
