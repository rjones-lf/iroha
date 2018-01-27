#!/usr/bin/env groovy

def doReleaseBuild() {
	def parallelism = env.PARALLELISM
	if ("arm7" in env.NODE_NAME) {
		parallelism = 1
	}
	def platform = sh(script: 'uname -m', returnStdout: true).trim()
	sh "curl -L -o /tmp/${env.GIT_COMMIT}/Dockerfile --create-dirs https://raw.githubusercontent.com/hyperledger/iroha/${env.GIT_COMMIT}/docker/develop/${platform}/Dockerfile"
	// pull docker image for building release package of Iroha
	// speeds up consequent image builds as we simply tag them 
	sh "docker pull ${DOCKER_BASE_IMAGE_DEVELOP}"
    iC = docker.build("hyperledger/iroha:${GIT_COMMIT}-${BUILD_NUMBER}", "--build-arg PARALLELISM=${parallelism} -f /tmp/${env.GIT_COMMIT}/Dockerfile /tmp/${env.GIT_COMMIT}")

	sh "mkdir /tmp/${env.GIT_COMMIT}-${BUILD_NUMBER} || true"
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

	    // copy build package to the volume
	    sh "cp ./build/iroha-*.deb /tmp/${GIT_COMMIT}/iroha.deb"
	}
	
	sh "curl -L -o /tmp/${env.GIT_COMMIT}/Dockerfile --create-dirs https://raw.githubusercontent.com/hyperledger/iroha/${env.GIT_COMMIT}/docker/release/${platform}/Dockerfile"
	sh "curl -L -o /tmp/${env.GIT_COMMIT}/entrypoint.sh https://raw.githubusercontent.com/hyperledger/iroha/${env.GIT_COMMIT}/docker/release/${platform}/entrypoint.sh"
	sh "cp /tmp/${GIT_COMMIT}-${BUILD_NUMBER}/iroha.deb /tmp/${env.GIT_COMMIT}"
	sh "chmod +x /tmp/${env.GIT_COMMIT}/entrypoint.sh"
    iCRelease = docker.build("hyperledger/iroha:${GIT_COMMIT}-${BUILD_NUMBER}-release", "-f /tmp/${env.GIT_COMMIT}/Dockerfile /tmp/${env.GIT_COMMIT}")
    docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-credentials') {
    	if (env.BRANCH_NAME == 'develop') {
    		iCRelease.push("${platform}-develop-latest")
    	}
    	else if (env.BRANCH_NAME == 'master') {
    		iCRelease.push("${platform}-latest")
    	}
    }
    sh "docker rmi ${iCRelease.id}"
}
return this
