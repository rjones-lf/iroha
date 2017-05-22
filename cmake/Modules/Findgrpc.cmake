find_path(grpc_INCLUDE_DIR grpc/grpc.h)
find_path(grpcpp_INCLUDE_DIR grpc++/grpc++.h)
find_library(grpc_LIB grpc)
find_library(grpcpp_LIB grpc++)
find_library(gpr_LIB gpr)

find_package(PackageHandleStandardArgs REQUIRED)
find_package_handle_standard_args(grpc
  FOUND_VAR grpc_FOUND
  REQUIRED_VARS grpc_INCLUDE_DIR grpcpp_INCLUDE_DIR grpc_LIB grpcpp_LIB gpr_LIB
  FAIL_MESSAGE "grpc is not found, it will be downloaded and installed"
  )
