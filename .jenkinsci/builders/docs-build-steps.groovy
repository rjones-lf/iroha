#!/usr/bin/env groovy

def buildSteps() {
    stage('Build docs'){
        def doxygen = load ".jenkinsci/utils/doxygen.groovy"
        def dPullOrBuild = load ".jenkinsci/docker-pull-or-build.groovy"
        def platform = sh(script: 'uname -m', returnStdout: true).trim()
        def iC = dPullOrBuild.dockerPullOrUpdate(
            "$platform-develop-build",
            "${env.GIT_RAW_BASE_URL}/${env.GIT_COMMIT}/docker/develop/Dockerfile",
            "${env.GIT_RAW_BASE_URL}/${env.GIT_PREVIOUS_COMMIT}/docker/develop/Dockerfile",
            "${env.GIT_RAW_BASE_URL}/dev/docker/develop/Dockerfile",
            ['PARALLELISM': params.PARALLELISM])
        iC.inside() {
            doxygen.doDoxygen()
        }
    }
}

return this