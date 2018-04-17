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

def dockerPullOrUpdate() {
  def platform = sh(script: 'uname -m', returnStdout: true).trim()
  def commit = sh(script: "echo ${BRANCH_NAME} | md5sum | cut -c 1-8", returnStdout: true).trim()
  if (remoteFilesDiffer("https://raw.githubusercontent.com/hyperledger/iroha/${env.GIT_COMMIT}/docker/develop/${platform}/Dockerfile", 
    "https://raw.githubusercontent.com/hyperledger/iroha/${env.GIT_PREVIOUS_COMMIT}/docker/develop/${platform}/Dockerfile")) {
    iC = docker.build("hyperledger/iroha:${commit}", "--build-arg PARALLELISM=${parallelism} -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")
    // develop branch Docker image has been modified
    if (BRANCH_NAME == 'develop') {
      docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-credentials') {
        iC.push("${platform}-develop")
      }
    }
  }
  else {
    // reuse develop branch Docker image
    if (BRANCH_NAME == 'develop') {
      iC = docker.image("hyperledger/iroha:${platform}-develop")
      iC.pull()
    }
    else {
      // first commit in this branch or Dockerfile modified
      if (remoteFilesDiffer("https://raw.githubusercontent.com/hyperledger/iroha/${env.GIT_COMMIT}/docker/develop/${platform}/Dockerfile", 
        "https://raw.githubusercontent.com/hyperledger/iroha/develop/docker/develop/${platform}/Dockerfile")) {
        iC = docker.build("hyperledger/iroha:${commit}", "--build-arg PARALLELISM=${parallelism} -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")
      }
      // reuse develop branch Docker image
      else {
        iC = docker.image("hyperledger/iroha:${platform}-develop")
      }
    }
  }
  return iC
}

return this
