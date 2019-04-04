#!/bin/bash
( /usr/bin/time -f "%S\t%U" g++-5 "${@}" 2> >(cat <(echo "g++-5 ${@}") - )) | sponge
