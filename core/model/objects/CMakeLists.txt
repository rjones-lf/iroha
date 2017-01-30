
# All domain instance is in libdomain.a
ADD_LIBRARY(objects STATIC
  asset.cpp
  domain.cpp
  message.cpp
  account.cpp
  peer.cpp
)
