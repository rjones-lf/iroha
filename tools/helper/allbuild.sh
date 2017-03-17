#!/bin/bash
./build.sh add
./build.sh transfer
./build.sh update
./build.sh remove
#./build.sh contract
rm -rf temp_*
