#!/usr/bin/env groovy

def doDebugBuild(coverageEnabled=false) {
	def parallelism = env.PARALLELISM
	if ("arm7" in env.NODE_NAME) {
		parallelism = 1
	}
	sh "docker network create ${env.IROHA_NETWORK}"

	docker.image('postgres:9.5').run(""
	    + " -e POSTGRES_USER=${env.IROHA_POSTGRES_USER}"
	    + " -e POSTGRES_PASSWORD=${env.IROHA_POSTGRES_PASSWORD}"
	    + " --name ${env.IROHA_POSTGRES_HOST}"
	    + " --network=${env.IROHA_NETWORK}")

	def platform = sh(script: 'uname -m', returnStdout: true).trim()
	sh "curl -L -o /tmp/${env.GIT_COMMIT}/Dockerfile --create-dirs https://raw.githubusercontent.com/hyperledger/iroha/${env.GIT_COMMIT}/docker/develop/${platform}/Dockerfile"
	// pull docker image in case we don't have one
	// speeds up consequent image builds as we simply tag them 
	sh "docker pull ${DOCKER_BASE_IMAGE_DEVELOP}"
	if (env.BRANCH_NAME == 'develop') {
		// --cache-from ${DOCKER_BASE_IMAGE_DEVELOP}
	    iC = docker.build("hyperledger/iroha:${GIT_COMMIT}-${BUILD_NUMBER}", "--build-arg PARALLELISM=${parallelism} -f /tmp/${env.GIT_COMMIT}/Dockerfile /tmp/${env.GIT_COMMIT}")
		docker.withRegistry('https://registry.hub.docker.com', 'docker-hub-credentials') {
        	iC.push("${platform}-develop")
        }
	}
	else {
	    iC = docker.build("hyperledger/iroha-workflow:${GIT_COMMIT}-${BUILD_NUMBER}", "-f /tmp/${env.GIT_COMMIT}/Dockerfile /tmp/${env.GIT_COMMIT} --build-arg PARALLELISM=${parallelism}")
	}
	iC.inside(""
	    + " -e IROHA_POSTGRES_HOST=${env.IROHA_POSTGRES_HOST}"
	    + " -e IROHA_POSTGRES_PORT=${env.IROHA_POSTGRES_PORT}"
	    + " -e IROHA_POSTGRES_USER=${env.IROHA_POSTGRES_USER}"
	    + " -e IROHA_POSTGRES_PASSWORD=${env.IROHA_POSTGRES_PASSWORD}"
	    + " --network=${env.IROHA_NETWORK}"
	    + " -v /var/jenkins/ccache:${CCACHE_DIR}") {

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
	          -DCOVERAGE=ON \
	          -DTESTING=ON \
	          -H. \
	          -Bbuild \
	          -DCMAKE_BUILD_TYPE=Debug \
	          -DIROHA_VERSION=${env.IROHA_VERSION}
	    """
	    sh "cmake --build build -- -j${parallelism}"
	    sh "ccache --show-stats"
	    sh "cmake --build build --target test"
	    sh "cmake --build build --target cppcheck"
        
        if ( coverageEnabled ) {
		    // Sonar
		    if (env.CHANGE_ID != null) {
		        sh """
		            sonar-scanner \
		                -Dsonar.github.disableInlineComments \
		                -Dsonar.github.repository='hyperledger/iroha' \
		                -Dsonar.analysis.mode=preview \
		                -Dsonar.login=${SONAR_TOKEN} \
		                -Dsonar.projectVersion=${BUILD_TAG} \
		                -Dsonar.github.oauth=${SORABOT_TOKEN} \
		                -Dsonar.github.pullRequest=${CHANGE_ID}
		        """
		    }

        	sh "lcov --capture --directory build --config-file .lcovrc --output-file build/reports/coverage_full.info"
		    sh "lcov --remove build/reports/coverage_full.info '/usr/*' 'schema/*' --config-file .lcovrc -o build/reports/coverage_full_filtered.info"
		    sh "python /tmp/lcov_cobertura.py build/reports/coverage_full_filtered.info -o build/reports/coverage.xml"                                
		    cobertura autoUpdateHealth: false, autoUpdateStability: false, coberturaReportFile: '**/build/reports/coverage.xml', conditionalCoverageTargets: '75, 50, 0', failUnhealthy: false, failUnstable: false, lineCoverageTargets: '75, 50, 0', maxNumberOfBuilds: 50, methodCoverageTargets: '75, 50, 0', onlyStable: false, zoomCoverageChart: false
        }

	    // TODO: replace with upload to artifactory server
        // develop branch only
        if ( env.BRANCH_NAME == "develop" ) {
            //archive(includes: 'build/bin/,compile_commands.json')
        }
	}
}
return this
