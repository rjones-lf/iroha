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
  // GIT_PREVIOUS_COMMIT is null for first PR build
  if (!previousDockerfileURL) {
    previousDockerfileURL = currentDockerfileURL
  }
  def commit = sh(script: "echo ${GIT_LOCAL_BRANCH} | md5sum | cut -c 1-8", returnStdout: true).trim()
  dockerImageFile = commit
  
  if (remoteFilesDiffer(currentDockerfileURL, previousDockerfileURL)) {
    // Dockerfile has been changed compared to the previous commit
    // Worst case scenario. We cannot count on the local cache
    // because Dockerfile may contain apt-get entries that would try to update
    // from invalid (stale) addresses
    iC = docker.build("${DOCKER_REGISTRY_BASENAME}:${commit}-${BUILD_NUMBER}", "${buildOptions} --no-cache -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")
  }
  else {
    // first commit in this branch or Dockerfile modified
    if (remoteFilesDiffer(currentDockerfileURL, referenceDockerfileURL)) {
      // if we're lucky to build on the same agent, image will be built using cache
      if ( env.NODE_NAME.contains('x86_64') ) {
        def testExitCode = sh(script: "docker load -i ${JENKINS_DOCKER_IMAGE_DIR}/${dockerImageFile}", returnStatus: true)
        if (testExitCode != 0) {
          sh "echo 'unable to load pre-built image for cache'"
        }
      }
      iC = docker.build("${DOCKER_REGISTRY_BASENAME}:${commit}-${BUILD_NUMBER}", "$buildOptions -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")
    }
    else {
      // try pulling image from Dockerhub, probably image is already there
      if ( env.NODE_NAME.contains('x86_64') ) {
        def testExitCode = sh(script: "docker load -i ${JENKINS_DOCKER_IMAGE_DIR}/${dockerImageFile}", returnStatus: true)
        if (testExitCode != 0) {
          // file with docker image was not found. Build it
          iC = docker.build("${DOCKER_REGISTRY_BASENAME}:${commit}-${BUILD_NUMBER}", "$buildOptions --no-cache -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")  
        }
        else {
          // file with docker image was found --> update from dockerhub (in case it is develop-build)
          iC = docker.image("${DOCKER_REGISTRY_BASENAME}:${imageName}")
          iC.pull()
        }
      }
      else {
        def testExitCode = sh(script: "docker pull ${DOCKER_REGISTRY_BASENAME}:${imageName}", returnStatus: true)
        if (testExitCode != 0) {
          // image does not (yet) exist on Dockerhub. Build it
          iC = docker.build("${DOCKER_REGISTRY_BASENAME}:${commit}-${BUILD_NUMBER}", "$buildOptions --no-cache -f /tmp/${env.GIT_COMMIT}/f1 /tmp/${env.GIT_COMMIT}")
        }
        else {
          iC = docker.image("${DOCKER_REGISTRY_BASENAME}:${imageName}")
        }
      }    
    }
  }
  if (GIT_LOCAL_BRANCH ==~ /develop|master/) {
    docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-credentials') {
      iC.push(imageName)
    }
  }
  // save results to the file
  if ( env.NODE_NAME.contains('x86_64') ) {
    dockerAgentImage = iC.imageName()
    sh "docker save -o /tmp/docker/${dockerImageFile} ${dockerAgentImage}"
  }
  return iC
}

return this
