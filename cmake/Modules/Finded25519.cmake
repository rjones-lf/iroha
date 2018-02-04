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
set(VERSION 56d926c4489aa7b2bb664a48294c5ee2d6e1c283)
set_target_description(ed25519 "Digital signature algorithm" ${URL} ${VERSION})

if (NOT ed25519_FOUND)
  externalproject_add(warchant_ed25519
      GIT_REPOSITORY ${URL}
      GIT_TAG        ${VERSION}
      CMAKE_ARGS     -DTESTING=OFF -DBUILD=STATIC
      INSTALL_COMMAND "" # remove install step
      TEST_COMMAND    "" # remove test step
      UPDATE_COMMAND  "" # remove update step
      )
  externalproject_get_property(warchant_ed25519 binary_dir)
  externalproject_get_property(warchant_ed25519 source_dir)
  set(ed25519_INCLUDE_DIR ${source_dir}/include)

  if(CMAKE_GENERATOR STREQUAL "Xcode")
    set(ed25519_LIBRARY ${binary_dir}/Debug/${CMAKE_STATIC_LIBRARY_PREFIX}ed25519${CMAKE_STATIC_LIBRARY_SUFFIX})
  else ()
    set(ed25519_LIBRARY ${binary_dir}/${CMAKE_STATIC_LIBRARY_PREFIX}ed25519${CMAKE_STATIC_LIBRARY_SUFFIX})
  endif ()
  
  file(MAKE_DIRECTORY ${ed25519_INCLUDE_DIR})
  link_directories(${binary_dir})

  add_dependencies(ed25519 warchant_ed25519)
endif ()

set_target_properties(ed25519 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${ed25519_INCLUDE_DIR}
    IMPORTED_LOCATION ${ed25519_LIBRARY}
    )

if(ENABLE_LIBS_PACKAGING)
  add_install_step_for_lib(${ed25519_LIBRARY})
endif()
