SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# All domain instance is in libdomain.a
ADD_LIBRARY(objects STATIC
  asset.cpp
  domain.cpp
  message.cpp
)
