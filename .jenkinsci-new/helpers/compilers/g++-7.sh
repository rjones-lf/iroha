#!/bin/bash
( /usr/bin/time -f "%S\t%U" g++-7 "${@}" 2> >(cat <(echo "g++-7 ${@}") - )) | sponge
