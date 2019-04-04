#!/bin/bash
( /usr/bin/time -f "%S\t%U" clang++-6.0 "${@}" 2> >(cat <(echo "clang++-6.0 ${@}") - )) | sponge
