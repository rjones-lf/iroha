#!/bin/bash
( /usr/bin/time -f "%S\t%U" clang++ "${@}" 2> >(cat <(echo "clang++ ${@}") - )) | sponge
