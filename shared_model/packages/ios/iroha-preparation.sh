#!/bin/bash

# download ios client and update dependencies
git clone https://github.com/hyperledger/iroha-ios.git
cd iroha-ios/
carthage update --platform iOS
cd SwiftyIrohaExample
carthage update --platform iOS

# build grpc client for sample application
cd grpc-swift/
rm -rf ./*
rm -rf ./.*
git clone --branch 0.3.3 https://github.com/grpc/grpc-swift.git .
make

# back to the root where script was executed
cd ../../..

# download and build Iroha library for iOS
curl https://raw.githubusercontent.com/hyperledger/iroha/master/shared_model/packages/ios/ios-build.sh > ios-build.sh

# optional step - sometimes connection timeout appears when using git: scheme instead of https url
sed -i '' 's|git://github.com/hyperledger/iroha-ed25519|https://github.com/hyperledger/iroha-ed25519.git|g' ios-build.sh

# build library
chmod +x ios-build.sh
./ios-build.sh SIMULATOR64 Debug

# place artifacts to proper sample's locations

# this command shows location for simulator artifacts
# use this command for device instead:
# mkdir -p iroha-ios/libs/iOS/
mkdir -p iroha-ios/libs/Simulator/

# this command shows location for simulator artifacts
# use this command for device instead:
# cp lib/* iroha-ios/libs/iOS/
cp lib/* iroha-ios/libs/Simulator/
cp -a include/. iroha-ios/headers/
