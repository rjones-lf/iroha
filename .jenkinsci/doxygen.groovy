#!/usr/bin/env groovy

def doDoxygen() {

    sh "doxygen Doxyfile"
    
    sshagent(['jenkins-artifact']) {
    sh "ssh-agent"
        sh "rsync -auzc docs/doxygen/html/* ubuntu@nexus.soramitsu.co.jp:/var/nexus-efs/doxygen/develop"
    }

    // if (env.GIT_LOCAL_BRANCH == 'master' || env.GIT_LOCAL_BRANCH == 'develop') {
    //     sshagent(['jenkins-artifact']) {
    //     sh "ssh-agent"
    //         sh "rsync -auzc docs/doxygen/html/* ubuntu@nexus.soramitsu.co.jp:/var/nexus-efs/doxygen/${env.GIT_LOCAL_BRANCH}"
    //         sh "rsync -auzc docs/doxygen/html/* ubuntu@nexus.soramitsu.co.jp:/var/nexus-efs/doxygen/${env.GIT_LOCAL_BRANCH}"
    //     }
    // }
}

return this