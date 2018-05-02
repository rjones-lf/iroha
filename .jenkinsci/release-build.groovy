#!/usr/bin/env groovy

def doReleaseBuild() {
  def parallelism = params.PARALLELISM
  // params are always null unless job is started
  // this is the case for the FIRST build only.
  // So just set this to same value as default. 
  // This is a known bug. See https://issues.jenkins-ci.org/browse/JENKINS-41929
  if (!parallelism) {
    parallelism = 4
  }
  if (env.NODE_NAME.contains('arm7')) {
    parallelism = 1
  }
  def platform = sh(script: 'uname -m', returnStdout: true).trim()
  sh "mkdir /tmp/${env.GIT_COMMIT}-${BUILD_NUMBER} || true"
  iC = docker.image("hyperledger/iroha:${platform}-develop-build")
  iC.pull()
  iC.inside(""
    + " -v /tmp/${GIT_COMMIT}-${BUILD_NUMBER}:/tmp/${GIT_COMMIT}"
    + " -v /var/jenkins/ccache:${CCACHE_RELEASE_DIR}") {

    def scmVars = checkout scm
    env.IROHA_VERSION = "0x${scmVars.GIT_COMMIT}"
    env.IROHA_HOME = "/opt/iroha"
    env.IROHA_BUILD = "${env.IROHA_HOME}/build"

    sh """
      ccache --version
      ccache --show-stats
      ccache --zero-stats
      ccache --max-size=5G
    """  
    sh """
      cmake \
        -H. \
        -Bbuild \
        -DCMAKE_BUILD_TYPE=Release \
        -DIROHA_VERSION=${env.IROHA_VERSION} \
        -DPACKAGE_DEB=ON \
        -DPACKAGE_TGZ=ON \
        -DCOVERAGE=OFF \
        -DTESTING=OFF
    """
    sh "cmake --build build --target package -- -j${parallelism}"
    sh "ccache --show-stats"

    // move build package to the volume
    sh "mv ./build/iroha-*.deb /tmp/${GIT_COMMIT}/iroha.deb"
    sh "mv ./build/*.tar.gz /tmp/${GIT_COMMIT}/iroha.tar.gz"
  }
  
  sh "curl -L -o /tmp/${env.GIT_COMMIT}/Dockerfile --create-dirs ${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/release/Dockerfile"
  sh "curl -L -o /tmp/${env.GIT_COMMIT}/entrypoint.sh ${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/release/entrypoint.sh"
  sh "mv /tmp/${GIT_COMMIT}-${BUILD_NUMBER}/iroha.deb /tmp/${env.GIT_COMMIT}"
  sh "chmod +x /tmp/${env.GIT_COMMIT}/entrypoint.sh"
  iCRelease = docker.build("hyperledger/iroha:${GIT_COMMIT}-${BUILD_NUMBER}-release", "--no-cache -f /tmp/${env.GIT_COMMIT}/Dockerfile /tmp/${env.GIT_COMMIT}")
  docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-credentials') {
    if (env.GIT_LOCAL_BRANCH == 'develop') {
      iCRelease.push("${platform}-develop")
    }
    else if (env.GIT_LOCAL_BRANCH == 'master') {
      iCRelease.push("${platform}-latest")
    }
  }
  sh "docker rmi ${iCRelease.id}"
}
return this
