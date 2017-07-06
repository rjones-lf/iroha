cmake_minimum_required(VERSION 3.0)
project(time64 C)

# Fixes time.h on 32 bit machites to work past the year 2038 bug
# version is from commit c17a49af2c8866f580ea05f44979e1266bc8b8c9
# this is only POSIX library
# https://github.com/evalEmpire/y2038
add_library(time64 SHARED
        time64.h
        time64.c
        time64_config.h
        time64_limits.h
        )

