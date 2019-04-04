#!/bin/bash
( /usr/bin/time -f "%S\t%U" clang++-7 "${@}" 2> >(cat <(echo "clang++-7 ${@}") - )) | sponge
