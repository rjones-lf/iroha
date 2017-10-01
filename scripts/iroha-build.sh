#!/bin/bash

cmake -H. -Bbuild
cmake --build build -- -j4

exit 0
