#!/bin/bash
( /usr/bin/time -f "%S\t%U" g++ "$@" 2> >(cat <(echo "g++ $@") - )) | sponge
