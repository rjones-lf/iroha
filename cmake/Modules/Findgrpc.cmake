add_library(grpc UNKNOWN IMPORTED)
add_library(grpc++ UNKNOWN IMPORTED)
add_library(gpr UNKNOWN IMPORTED)
add_executable(grpc_cpp_plugin IMPORTED)

find_path(grpc_INCLUDE_DIR grpc/grpc.h)
mark_as_advanced(grpc_INCLUDE_DIR)

find_library(grpc_LIBRARY grpc)
mark_as_advanced(grpc_LIBRARY)

find_library(grpc_grpc++_LIBRARY grpc++)
mark_as_advanced(grpc_grpc++_LIBRARY)

find_library(gpr_LIBRARY gpr)
mark_as_advanced(gpr_LIBRARY)

find_program(grpc_CPP_PLUGIN grpc_cpp_plugin)
mark_as_advanced(grpc_CPP_PLUGIN)

find_package_handle_standard_args(grpc DEFAULT_MSG
    grpc_LIBRARY
    grpc_INCLUDE_DIR
    gpr_LIBRARY
    grpc_CPP_PLUGIN
    )

set(URL https://github.com/grpc/grpc)
set(VERSION bfcbad3b86c7912968dc8e64f2121c920dad4dfb)
set_target_description(grpc "Remote Procedure Call library" ${URL} ${VERSION})

if (NOT grpc_FOUND)
  find_package(Git REQUIRED)
  externalproject_add(grpc_grpc
      GIT_REPOSITORY ${URL}
      GIT_TAG        ${VERSION}
      CMAKE_ARGS
        -DgRPC_PROTOBUF_PROVIDER=package
        -DgRPC_PROTOBUF_PACKAGE_TYPE=CONFIG
        -DProtobuf_DIR=${EP_PREFIX}/src/google_protobuf-build/lib/cmake/protobuf
        -DgRPC_ZLIB_PROVIDER=package
        -DBUILD_SHARED_LIBS=ON
      PATCH_COMMAND
        ${GIT_EXECUTABLE} apply ${PROJECT_SOURCE_DIR}/patch/fix-protobuf-package-include.patch || true
      BUILD_BYPRODUCTS
        ${EP_PREFIX}/src/grpc_grpc-build/grpc_cpp_plugin
        ${EP_PREFIX}/src/grpc_grpc-build/${CMAKE_SHARED_LIBRARY_PREFIX}gpr${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${EP_PREFIX}/src/grpc_grpc-build/${CMAKE_SHARED_LIBRARY_PREFIX}grpc${CMAKE_SHARED_LIBRARY_SUFFIX}
        ${EP_PREFIX}/src/grpc_grpc-build/${CMAKE_SHARED_LIBRARY_PREFIX}grpc++${CMAKE_SHARED_LIBRARY_SUFFIX}
      INSTALL_COMMAND "" # remove install step
      TEST_COMMAND "" # remove test step
      UPDATE_COMMAND "" # remove update step
      )
  externalproject_get_property(grpc_grpc source_dir binary_dir)
  set(grpc_INCLUDE_DIR ${source_dir}/include)
  set(gpr_LIBRARY ${binary_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}gpr${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(grpc_LIBRARY ${binary_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}grpc${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(grpc_grpc++_LIBRARY ${binary_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}grpc++${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(grpc_CPP_PLUGIN ${binary_dir}/grpc_cpp_plugin)
  file(MAKE_DIRECTORY ${grpc_INCLUDE_DIR})
  link_directories(${binary_dir})

  add_dependencies(grpc_grpc protobuf)
  add_dependencies(grpc grpc_grpc)
  add_dependencies(grpc++ grpc_grpc)
  add_dependencies(grpc_cpp_plugin grpc_grpc protoc)
endif ()

set_target_properties(grpc PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${grpc_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES "pthread;dl"
    IMPORTED_LOCATION ${grpc_LIBRARY}
    )

set_target_properties(grpc++ PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${grpc_INCLUDE_DIR}
    INTERFACE_LINK_LIBRARIES grpc
    IMPORTED_LOCATION ${grpc_grpc++_LIBRARY}
    )

set_target_properties(grpc_cpp_plugin PROPERTIES
    IMPORTED_LOCATION ${grpc_CPP_PLUGIN}
    )

set_target_properties(gpr PROPERTIES
	IMPORTED_LOCATION ${gpr_LIBRARY}
	)

if(ENABLE_LIBS_PACKAGING)
  add_install_step_for_lib(${grpc_LIBRARY})
  add_install_step_for_lib(${grpc_grpc++_LIBRARY})
  add_install_step_for_lib(${gpr_LIBRARY})
endif()
