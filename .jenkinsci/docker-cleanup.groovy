#!/usr/bin/env groovy

// remove docker network and stale images
def doDockerCleanup() {
  sh """
    # Check whether the image is the last-standing man
    # i.e., no other tags exist for this image
    docker rmi \$(docker images --no-trunc --format '{{.Repository}}:{{.Tag}}\\t{{.ID}}' | grep \$(docker images --no-trunc --format '{{.ID}}' ${iC.id}) | head -n -1 | cut -f 1) || true
  """
}

// cleanup docker network created for the test stage
def doDockerNetworkCleanup() {
  sh "docker network rm ${env.IROHA_NETWORK}"
}

// cleanup docker images which weren't used for more that 20 days and image for this PR in case of successful PR
def doStaleDockerImagesCleanup() {
  sh "find ${JENKINS_DOCKER_IMAGE_DIR} -type f -mtime +20 -exec rm -f {} \\;"
  sh "rm -f ${JENKINS_DOCKER_IMAGE_DIR}/${DOCKER_IMAGE_FILE}"
}

return this
