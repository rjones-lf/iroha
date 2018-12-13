#!/usr/bin/env groovy

def doDoxygen() {
  sh "doxygen Doxyfile"
  // TODO: Remove this comment once dev branch will return to develop
  // I will not be changing branches here. It requires some rewriting
  // Hope dev branch situation will be resolved soon
  if (env.GIT_LOCAL_BRANCH in ["master","develop"] || env.CHANGE_BRANCH_LOCAL == 'develop') {
    def branch = env.CHANGE_BRANCH_LOCAL == 'develop' ? env.CHANGE_BRANCH_LOCAL : env.GIT_LOCAL_BRANCH
    sshagent(['jenkins-artifact']) {
      sh "ssh-agent"
      sh """
        rsync \
        -e 'ssh -o UserKnownHostsFile=/dev/null -o StrictHostKeyChecking=no' \
        -rzcv --delete \
        docs/doxygen/html/* \
        ubuntu@docs.iroha.tech:/var/nexus-efs/doxygen/${branch}/
      """
    }
  } else {
    archiveArtifacts artifacts: 'docs/doxygen/html/*', allowEmptyArchive: true
  }
}

return this
