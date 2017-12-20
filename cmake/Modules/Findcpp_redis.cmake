add_library(cpp_redis UNKNOWN IMPORTED)

find_path(cpp_redis_INCLUDE_DIR cpp_redis/cpp_redis)
mark_as_advanced(cpp_redis_INCLUDE_DIR)

find_library(cpp_redis_LIBRARY cpp_redis)
mark_as_advanced(cpp_redis_LIBRARY)

find_library(tacopie_LIBRARY tacopie)
mark_as_advanced(cpp_redis_LIBRARY)

find_package_handle_standard_args(cpp_redis DEFAULT_MSG
    cpp_redis_INCLUDE_DIR
    cpp_redis_LIBRARY
    tacopie_LIBRARY
    )

set(URL1 https://github.com/Cylix/cpp_redis.git)
set(VERSION1 f390eef447a62dcb6da288fb1e91f25f8a9b838c)
set_target_description(cpp_redis "C++ redis client" ${URL1} ${VERSION1})

set(URL2 https://github.com/Cylix/tacopie.git)
set(VERSION2 4c551b8ff1c53c5fa63286371c9c884254fc9423)
set_target_description(tacopie "C++ tcp library" ${URL2} ${VERSION2})

if (NOT cpp_redis_FOUND)
  externalproject_add(cylix_cpp_redis
      GIT_REPOSITORY ${URL1}
      GIT_TAG        ${VERSION1}
      CMAKE_ARGS -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
      BUILD_BYPRODUCTS ${EP_PREFIX}/src/cylix_cpp_redis-build/lib/libcpp_redis.a
      INSTALL_COMMAND "" # remove install step
      UPDATE_COMMAND "" # remove update step
      TEST_COMMAND "" # remove test step
      )
  externalproject_get_property(cylix_cpp_redis source_dir binary_dir)
  set(tacopie_SOURCE_DIR ${source_dir}/tacopie)
  set(cpp_redis_INCLUDE_DIR ${source_dir}/includes)
  set(cpp_redis_LIBRARY ${binary_dir}/lib/libcpp_redis.a)
  set(tacopie_LIBRARY ${binary_dir}/lib/libtacopie.a)
  file(MAKE_DIRECTORY ${cpp_redis_INCLUDE_DIR})

  externalproject_add(cylix_tacopie
      GIT_REPOSITORY  ${URL2}
      GIT_TAG         ${VERSION2}
      SOURCE_DIR ${tacopie_SOURCE_DIR}
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      BUILD_BYPRODUCTS ${EP_PREFIX}/src/cylix_cpp_redis-build/lib/libtacopie.a
      INSTALL_COMMAND "" # remove install step
      UPDATE_COMMAND "" # remove update step
      TEST_COMMAND "" # remove test step
      )

  add_dependencies(cylix_cpp_redis cylix_tacopie)
  add_dependencies(cpp_redis cylix_cpp_redis)
endif ()

set_target_properties(cpp_redis PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${cpp_redis_INCLUDE_DIR}
    IMPORTED_LOCATION ${cpp_redis_LIBRARY}
    INTERFACE_LINK_LIBRARIES "${tacopie_LIBRARY};pthread"
    )
