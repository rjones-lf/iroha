#!/usr/bin/env groovy

def remoteFilesDiffer(f1, f2) {
  sh "curl -L -o /tmp/${env.GIT_COMMIT}/f1 --create-dirs ${f1}"
  sh "curl -L -o /tmp/${env.GIT_COMMIT}/f2 ${f2}"
  diffExitCode = sh(script: "diff -q /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}/f2", returnStatus: true)
  return diffExitCode != 0
}

def buildOptionsString(options) {
  def s = ''
  if (options) {
    options.each { k, v ->
      s += "--build-arg ${k}=${v} "
    }
  }
  return s
}

def dockerPullOrUpdate(imageName, currentDockerfileURL, previousDockerfileURL, referenceDockerfileURL, buildOptions=null) {
  buildOptions = buildOptionsString(buildOptions)
  def uploadExitCode = 0
  // GIT_PREVIOUS_COMMIT is null for first PR build
  def commit = sh(script: "echo ${GIT_LOCAL_BRANCH} | md5sum | cut -c 1-8", returnStdout: true).trim()
  DOCKER_IMAGE_FILE = commit
  
  if (remoteFilesDiffer(currentDockerfileURL, previousDockerfileURL)) {
    // Dockerfile has been changed compared to the previous commit
    // Worst case scenario. We cannot count on the local cache
    // because Dockerfile may contain apt-get entries that would try to update
    // from invalid (stale) addresses
    iC = docker.build("${DOCKER_REGISTRY_BASENAME}:${commit}-${BUILD_NUMBER}", "${buildOptions} --no-cache -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")
    if (env.NODE_NAME.contains('x86_64')) {
      sh "docker save -o ${JENKINS_DOCKER_IMAGE_DIR}/${DOCKER_IMAGE_FILE} ${DOCKER_REGISTRY_BASENAME}:${commit}-${BUILD_NUMBER}"
    }
  }
  else {
    // upload reference image (hyperledger/iroha:develop-build) (required in all use-cases in this execution branch)
    if (env.NODE_NAME.contains('x86_64')) {
      uploadExitCode = sh(script: "docker load -i ${JENKINS_DOCKER_IMAGE_DIR}/${imageName}", returnStatus: true)
      if (uploadExitCode != 0) {
        sh "echo 'Reference image ${DOCKER_REGISTRY_BASENAME}:${imageName} doesn't exist on the EFS"
      }
    }
    def pullExitCode = sh(script: "docker pull ${DOCKER_REGISTRY_BASENAME}:${imageName}", returnStatus: true)
    if (pullExitCode != 0) {
      sh "echo 'Reference image ${DOCKER_REGISTRY_BASENAME}:${imageName} doesn't exist on the dockerhub"
    }
    else {
      // save reference docker image into the file when it is doesn't exist on the EFS
      if (uploadExitCode != 0) {
        sh "docker save -o ${JENKINS_DOCKER_IMAGE_DIR}/${imageName} ${DOCKER_REGISTRY_BASENAME}:${imageName}"
      }
    }
    // first commit in this branch or Dockerfile modified
    if (remoteFilesDiffer(currentDockerfileURL, referenceDockerfileURL)) {
      // if we're lucky to build on the same agent, image will be built using cache
      // for x86 download reference image and start build
      iC = docker.build("${DOCKER_REGISTRY_BASENAME}:${commit}-${BUILD_NUMBER}", "${buildOptions} -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")
      if (env.NODE_NAME.contains('x86_64')) {
        sh "docker save -o /tmp/docker/${imageName} ${DOCKER_REGISTRY_BASENAME}:${commit}-${BUILD_NUMBER}"
      }
    }
    else {
      // now we get reference image from the file (pull from dockerhub)
      iC = docker.image("${DOCKER_REGISTRY_BASENAME}:${imageName}")
      DOCKER_IMAGE_FILE = imageName
    }
  }
  if (GIT_LOCAL_BRANCH ==~ /develop|master/) {
    docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-credentials') {
      iC.push(imageName)
    }
  }
  DOCKER_AGENT_IMAGE = iC.imageName()
  return iC
}

return this
