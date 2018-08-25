#!/usr/bin/env groovy

def doJavaBindings(os, packageName, buildType=Release) {
  def currentPath = sh(script: "pwd", returnStdout: true).trim()
  def commit = env.GIT_COMMIT
  def artifactsPath = sprintf('%1$s/java-bindings-%2$s-%3$s-%4$s-%5$s.zip',
    [currentPath, buildType, os, sh(script: 'date "+%Y%m%d"', returnStdout: true).trim(), commit.substring(0,6)])
  def cmakeOptions = ""
  if (os == 'windows') {
    sh "mkdir -p /tmp/${commit}/bindings-artifact"
    cmakeOptions = '-DCMAKE_TOOLCHAIN_FILE=/c/Users/Administrator/Downloads/vcpkg-master/vcpkg-master/scripts/buildsystems/vcpkg.cmake -G "NMake Makefiles"'
  }
  if (os == 'linux') {
    // do not use preinstalled libed25519
    sh "rm -rf /usr/local/include/ed25519*; unlink /usr/local/lib/libed25519.so; rm -f /usr/local/lib/libed25519.so.1.2.2"
  }
  sh """
    cmake \
      -Hshared_model \
      -Bbuild \
      -DCMAKE_BUILD_TYPE=$buildType \
      -DSWIG_JAVA=ON \
      -DSWIG_JAVA_PKG="$packageName" \
      ${cmakeOptions}
  """
  def parallelismParam = (os == 'windows') ? '' : "-j${params.PARALLELISM}"
  sh "cmake --build build --target irohajava -- ${parallelismParam}"
  // TODO 29.05.18 @bakhtin Java tests never finishes on Windows Server 2016. IR-1380
  sh "pushd build/bindings; \
      zip -r $artifactsPath *.dll *.lib *.manifest *.exp libirohajava.so \$(echo ${packageName} | cut -d '.' -f1); \
      popd"
  if (os == 'windows') {
    sh "cp $artifactsPath /tmp/${commit}/bindings-artifact"
  }
  else {
    sh "cp $artifactsPath /tmp/bindings-artifact"
  }
  return artifactsPath
}

def doPythonBindings(os, buildType=Release) {
  def currentPath = sh(script: "pwd", returnStdout: true).trim()
  def commit = env.GIT_COMMIT
  def supportPython2 = "OFF"
  def artifactsPath = sprintf('%1$s/python-bindings-%2$s-%3$s-%4$s-%5$s-%6$s.zip',
    [currentPath, env.PBVersion, buildType, os, sh(script: 'date "+%Y%m%d"', returnStdout: true).trim(), commit.substring(0,6)])
  def cmakeOptions = ""

  sh "mkdir -p /tmp/${commit}/bindings-artifact"
  if (os == 'windows') {
    cmakeOptions = """ \\
      -DPYTHON_INCLUDE_DIR=/c/users/administrator/anaconda64/envs/py3.5/include/ \\
      -DPYTHON_LIBRARY=/c/users/administrator/anaconda64/envs/py3.5/libs/python35.lib \\
      -DPYTHON_EXECUTABLE=/c/users/administrator/anaconda64/envs/py3.5/python \\
      -DCMAKE_TOOLCHAIN_FILE=/c/Users/Administrator/Downloads/vcpkg-master/vcpkg-master/scripts/buildsystems/vcpkg.cmake -G "NMake Makefiles" \\
    """
  }
  if (os == 'mac') {
    cmakeOptions = """ \\
      -DPYTHON_INCLUDE_DIR=/Users/jenkins/.pyenv/versions/3.5.5/include/python3.5m/ \\
      -DPYTHON_LIBRARY=/Users/jenkins/.pyenv/versions/3.5.5/lib/libpython3.5m.a \\
      -DPYTHON_EXECUTABLE=/Users/jenkins/.pyenv/versions/3.5.5/bin/python3.5 \\
    """
  }
  if (os == 'linux') {
    cmakeOptions = """ \\
      -DPYTHON_INCLUDE_DIR=/home/iroha-ci/.pyenv/versions/3.5.5/include/python3.5m/ \\
      -DPYTHON_LIBRARY=/home/iroha-ci/.pyenv/versions/3.5.5/lib/libpython3.5m.a \\
      -DPYTHON_EXECUTABLE=/home/iroha-ci/.pyenv/versions/3.5.5/bin/python3.5 \\
    """

    // do not use preinstalled libed25519
    sh "rm -rf /usr/local/include/ed25519*; unlink /usr/local/lib/libed25519.so; rm -f /usr/local/lib/libed25519.so.1.2.2"
  }
  if (env.PBVersion == "python2") { supportPython2 = "ON" }
  sh """
    cmake \
      -Hshared_model \
      -Bbuild \
      -DCMAKE_BUILD_TYPE=${buildType} \
      -DSWIG_PYTHON=ON \
      -DSUPPORT_PYTHON2=${supportPython2} \
      ${cmakeOptions}
  """
  def parallelismParam = (os == 'windows') ? '' : "-j${params.PARALLELISM}"
  sh "cmake --build build --target irohapy -- ${parallelismParam}"
  sh "cmake --build build --target python_tests"
  sh "cd build; ctest -R python --output-on-failure"
  if (os ==~ /(mac|linux)/) {
    sh """
      protoc --proto_path=shared_model/schema \
        --python_out=build/bindings shared_model/schema/*.proto; \
      python -m grpc_tools.protoc --proto_path=shared_model/schema --python_out=build/bindings \
        --grpc_python_out=build/bindings \
        shared_model/schema/endpoint.proto
    """
  }
  if (os == 'windows') {
    sh """
      protoc --proto_path=shared_model/schema \
        --proto_path=/c/Users/Administrator/Downloads/vcpkg-master/vcpkg-master/buildtrees/protobuf/src/protobuf-3.5.1-win32/include \
        --python_out=build/bindings shared_model/schema/*.proto; \
      python -m grpc_tools.protoc \
        --proto_path=/c/Users/Administrator/Downloads/vcpkg-master/vcpkg-master/buildtrees/protobuf/src/protobuf-3.5.1-win32/include \
        --proto_path=shared_model/schema --python_out=build/bindings --grpc_python_out=build/bindings \
        shared_model/schema/endpoint.proto
    """
  }

  sh """ 
    zip -jq ${artifactsPath} build/bindings/*.{py,dll,so,pyd,lib,dll,exp,manifest} || true; \
    cp ${artifactsPath} /tmp/${commit}/bindings-artifact
  """
  
  return artifactsPath
}

def doAndroidBindings(abiVersion) {
  def currentPath = sh(script: "pwd", returnStdout: true).trim()
  def commit = env.GIT_COMMIT
  def artifactsPath = sprintf('%1$s/android-bindings-%2$s-%3$s-%4$s-%5$s-%6$s.zip',
    [currentPath, "\$PLATFORM", abiVersion, "\$BUILD_TYPE_A", sh(script: 'date "+%Y%m%d"', returnStdout: true).trim(), commit.substring(0,6)])
  sh """
    (cd /iroha; git init; git remote add origin https://github.com/hyperledger/iroha.git; \
    git fetch origin ${GIT_COMMIT}; git checkout FETCH_HEAD)
  """
  sh """
    . /entrypoint.sh; \
    sed -i.bak "s~find_package(JNI REQUIRED)~SET(CMAKE_SWIG_FLAGS \\\${CMAKE_SWIG_FLAGS} -package \${PACKAGE})~" /iroha/shared_model/bindings/CMakeLists.txt; \
    # TODO: might not be needed in the future
    sed -i.bak "/target_include_directories(\\\${SWIG_MODULE_irohajava_REAL_NAME} PUBLIC/,+3d" /iroha/shared_model/bindings/CMakeLists.txt; \
    sed -i.bak "s~swig_link_libraries(irohajava~swig_link_libraries(irohajava \"/protobuf/.build/lib\${PROTOBUF_LIB_NAME}.a\" \"\${NDK_PATH}/platforms/android-$abiVersion/\${ARCH}/usr/\${LIBP}/liblog.so\"~" /iroha/shared_model/bindings/CMakeLists.txt; \
    sed -i.bak "s~find_library(protobuf_LIBRARY protobuf)~find_library(protobuf_LIBRARY \${PROTOBUF_LIB_NAME})~" /iroha/cmake/Modules/Findprotobuf.cmake; \
    sed -i.bak "s~find_program(protoc_EXECUTABLE protoc~set(protoc_EXECUTABLE \"/protobuf/host_build/protoc\"~" /iroha/cmake/Modules/Findprotobuf.cmake; \
    cmake -H/iroha/shared_model -B/iroha/shared_model/build -DCMAKE_SYSTEM_NAME=Android -DCMAKE_SYSTEM_VERSION=$abiVersion -DCMAKE_ANDROID_ARCH_ABI=\$PLATFORM \
      -DANDROID_NDK=\$NDK_PATH -DCMAKE_ANDROID_STL_TYPE=c++_static -DCMAKE_BUILD_TYPE=\$BUILD_TYPE_A -DTESTING=OFF \
      -DSWIG_JAVA=ON -DCMAKE_PREFIX_PATH=\$DEPS_DIR
    """
  sh "cmake --build /iroha/shared_model/build --target irohajava -- -j${params.PARALLELISM}"
  sh "zip -j $artifactsPath /iroha/shared_model/build/bindings/*.java /iroha/shared_model/build/bindings/libirohajava.so"
  sh "cp $artifactsPath /tmp/bindings-artifact"
  return artifactsPath
}

return this
