#!/bin/bash

cd cmake

if ! grep -q PATCH_COMMAND dependencies.cmake; then
  sed -i '/https:\/\/github.com\/gvanas\/KeccakCodePackage.git/a\ \ PATCH_COMMAND     bash -c "sed -i \\"s/KeccakWidth1600_Sponge/KeccakWidth800_Sponge/\\" Modes/SimpleFIPS202.c"' \
    dependencies.cmake

  sed -i -e 's/KeccakP800_excluded/KeccakP1600_excluded/' \
    -e 's/amd64/arm/' \
    -e 's/generic64/generic32/' \
    dependencies.cmake
fi

cd ..

if ! grep -q generic32 fabfile.py; then
  sed -i -e 's/generic64/generic32/' \
    -e 's/amd64/armhf/' fabfile.py
fi

exit 0
