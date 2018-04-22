#!/usr/bin/env groovy

def remoteFilesDiffer(f1, f2) {
  sh "curl -L -o /tmp/${env.GIT_COMMIT}/f1 --create-dirs ${f1}"
  sh "curl -L -o /tmp/${env.GIT_COMMIT}/f2 ${f2}"
  diffExitCode = sh(script: "diff -q /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}/f2", returnStatus: true)
  if (diffExitCode == 0) {
    return false
  }
  return true
}

def buildOptionsString(options) {
  def s = ''
  if (options) {
    options.each { k, v ->
      s += "--build-arg ${k}=${v} "
    }
    return s
  }
  return ''
}

def dockerPullOrUpdate(imageName, currentDockerfileURL, previousDockerfileURL, referenceDockerfileURL, buildOptions=null) {
  buildOptions = buildOptionsString(buildOptions)
  def commit = sh(script: "echo ${BRANCH_NAME} | md5sum | cut -c 1-8", returnStdout: true).trim()
  if (remoteFilesDiffer(currentDockerfileURL, previousDockerfileURL)) {
    // Dockerfile has been changed compared to the previous commit
    // Worst case scenario. We cannot count on the local cache
    // because Dockerfile may contain apt-get entries that would try to update
    // from invalid (stale) addresses
    iC = docker.build("hyperledger/iroha:${commit}-${BUILD_NUMBER}", "${buildOptions} --no-cache -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")
  }
  else {
    // first commit in this branch or Dockerfile modified
    if (remoteFilesDiffer(currentDockerfileURL, referenceDockerfileURL)) {
      // if we're lucky to build on the same agent, image will be built using cache
      iC = docker.build("hyperledger/iroha:${commit}-${BUILD_NUMBER}", "$buildOptions -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")
    }
    else {
      // try pulling image from Dockerhub, probably image is already there
      def testExitCode = sh(script: "docker pull hyperledger/iroha:${imageName}", returnStatus: true)
      if (testExitCode != 0) {
        // image does not (yet) exist on Dockerhub. Build it
        iC = docker.build("hyperledger/iroha:${commit}-${BUILD_NUMBER}", "$buildOptions --no-cache -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")   
      }
      else {
        // no difference found compared to both previous and reference Dockerfile
        iC = docker.image("hyperledger/iroha:${imageName}")
      }
    }
  }
  if (BRANCH_NAME ==~ /develop|master/) {
    docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-credentials') {
      iC.push(imageName)
    }
  }
  return iC
}

return this
