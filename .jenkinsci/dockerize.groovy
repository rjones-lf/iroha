#!/usr/bin/env groovy

def doDockerize() {
    
    sh "cp ${IROHA_BUILD}/iroha.deb ${IROHA_RELEASE}/iroha.deb"

    env.TAG = ""
    if (env.CHANGE_ID != null) {
        env.TAG = env.CHANGE_ID
    }
    else {
        if ( env.BRANCH_NAME == "develop" ) {
            env.TAG = "develop"
        }
        elif ( env.BRANCH_NAME == "master" ) {
            env.TAG = "latest"
        }
    }

    sh """
    echo TAG IS ${env.TAG}
    docker login -u ${DOCKERHUB_USR} -p ${DOCKERHUB_PSW}
    docker build -t hyperledger/iroha-docker:${TAG} ${IROHA_RELEASE}
    docker push hyperledger/iroha-docker:${TAG}
    """
}
