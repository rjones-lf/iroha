// Overall pipeline looks like the following
//               |----Debug
//   |--Linux----|----Release
//   | 
//   |    OR
//   |           
//-- |--ARM------|----Debug
//   |           |----Release
//   |    OR
//   |
//   |--MacOS----|----Debug
//   |           |----Release
properties([parameters([
    choice(choices: 'Debug\nRelease', description: '', name: 'BUILD_TYPE'),
    booleanParam(defaultValue: true, description: '', name: 'Linux'),
    booleanParam(defaultValue: false, description: '', name: 'ARM'),
    booleanParam(defaultValue: false, description: '', name: 'MacOS'),
    string(defaultValue: '4', description: 'How much parallelism should we exploit. "4" is optimal for machines with modest amount of memory and at least 4 cores', name: 'PARALLELISM')]),
    pipelineTriggers([cron('@weekly')])])
pipeline {
    environment {
        SORABOT_TOKEN = credentials('SORABOT_TOKEN')
        SONAR_TOKEN = credentials('SONAR_TOKEN')
        CODECOV_TOKEN = credentials('CODECOV_TOKEN')
        DOCKERHUB = credentials('DOCKERHUB')
        DOCKER_IMAGE = 'hyperledger/iroha-docker-develop:v1'        

        IROHA_NETWORK = "iroha-${GIT_COMMIT}"
        IROHA_POSTGRES_HOST = "pg-${GIT_COMMIT}"
        IROHA_POSTGRES_USER = "pg-user-${GIT_COMMIT}"
        IROHA_POSTGRES_PASSWORD = "${GIT_COMMIT}"
        IROHA_REDIS_HOST = "redis-${GIT_COMMIT}"
        IROHA_POSTGRES_PORT = 5432
        IROHA_REDIS_PORT = 6379
    }
    agent {
        label 'docker_1'
    }
    stages {
        stage('Build Debug') {
            when { expression { params.BUILD_TYPE == 'Debug' } }
            parallel {
                stage ('Linux') {
                    when { expression { return params.Linux } }
                    steps {
                        script {
                            def p_c = docker.image('postgres:9.5').run(""
                                + "-e POSTGRES_USER=${env.IROHA_POSTGRES_USER}"
                                + "-e POSTGRES_PASSWORD=${env.IROHA_POSTGRES_PASSWORD}"
                                + "--name ${env.IROHA_POSTGRES_HOST}"
                                + "--network=${env.IROHA_NETWORK}")

                            def r_c = docker.image('redis:3.2.8').run(""
                                + "--name ${env.IROHA_REDIS_HOST}"
                                + "--network=${env.IROHA_NETWORK}")

                            docker.image("${env.DOCKER_IMAGE}").inside(""
                                + "-e IROHA_POSTGRES_HOST=${env.IROHA_POSTGRES_HOST}"
                                + "-e IROHA_POSTGRES_PORT=${env.IROHA_POSTGRES_PORT}" 
                                + "-e IROHA_POSTGRES_USER=${env.IROHA_POSTGRES_USER}" 
                                + "-e IROHA_POSTGRES_PASSWORD=${env.IROHA_POSTGRES_PASSWORD}"
                                + "-e IROHA_REDIS_HOST=${env.IROHA_REDIS_HOST}" 
                                + "-e IROHA_REDIS_PORT=${env.IROHA_REDIS_PORT}" 
                                + "--network=${env.IROHA_NETWORK}") {

                            def scmVars = checkout scm
                            env.IROHA_VERSION = "0x${scmVars.GIT_COMMIT}"
                            env.IROHA_HOME = "/opt/iroha"
                            env.IROHA_BUILD = "/opt/iroha/build"
                            env.IROHA_RELEASE = "${env.IROHA_HOME}/docker/release"

                            sh """
                                ccache --version
                                ccache --show-stats
                                ccache --zero-stats
                                ccache --max-size=1G
                            """
                            sh """
                                cmake \
                                  -DCOVERAGE=ON \
                                  -DTESTING=ON \
                                  -H. \
                                  -Bbuild \
                                  -DCMAKE_BUILD_TYPE=${params.BUILD_TYPE} \
                                  -DIROHA_VERSION=${env.IROHA_VERSION}
                            """
                            sh "cmake --build build -- -j${params.PARALLELISM}"
                            sh "ccache --cleanup"
                            sh "ccache --show-stats"

                            sh "cmake --build build --target test"
                            sh "cmake --build build --target gcovr"
                            sh "cmake --build build --target cppcheck"

                            // Dockerize
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

                            // do it for master or develop branches

                            if ( env.BRANCH_NAME == "master" ||
                                 env.BRANCH_NAME == "develop" ) {

                                sh "docker login -u ${DOCKERHUB_USR} -p ${DOCKERHUB_PSW}"
                                sh "docker build -t hyperledger/iroha-docker:${TAG} ${IROHA_RELEASE}"
                                sh "docker push hyperledger/iroha-docker:${TAG}"
                            }
                            
                            // Codecov
                            sh "bash <(curl -s https://codecov.io/bash) -f build/reports/gcovr.xml -t ${CODECOV_TOKEN} || echo 'Codecov did not collect coverage reports'"

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

                            //stash(allowEmpty: true, includes: 'build/compile_commands.json', name: 'Compile commands')
                            //stash(allowEmpty: true, includes: 'build/reports/', name: 'Reports')
                            archive(includes: 'build/bin/,compile_commands.json')
                            }
                        }
                    }
                }
                stage('ARM') {
                    when { expression { return params.ARM } }
                    steps {
                        sh "echo ARM build will be running there"    
                    }                    
                }
                stage('MacOS'){
                    when { expression { return  params.MacOS } }
                    steps {
                        sh "MacOS build will be running there"
                    }
                }
            }
        }
        stage('Build Release') {
            when { expression { params.BUILD_TYPE == 'Release' } }
            parallel {
                stage('Linux') {
                    when { expression { return params.Linux } }
                    steps {
                        script {
                            def scmVars = checkout scm
                            env.IROHA_VERSION = "0x${scmVars.GIT_COMMIT}"
                        }
                        sh """
                            ccache --version
                            ccache --show-stats
                            ccache --zero-stats
                            ccache --max-size=1G
                        """
                        sh """
                            cmake \
                              -DCOVERAGE=OFF \
                              -DTESTING=OFF \
                              -H. \
                              -Bbuild \
                              -DCMAKE_BUILD_TYPE=${params.BUILD_TYPE} \
                              -DPACKAGE_DEB=ON \
                              -DPACKAGE_TGZ=ON \
                              -DIROHA_VERSION=${IROHA_VERSION}
                        """
                        sh "cmake --build build -- -j${params.PARALLELISM}"
                        sh "ccache --cleanup"
                        sh "ccache --show-stats"
                        sh """
                        mv build/iroha-{*,linux}.deb && mv build/iroha-{*,linux}.tar.gz
                        echo ${IROHA_VERSION} > version.txt
                        """
                        archive(includes: 'build/iroha-linux.deb,build/iroha-linux.tar.gz,build/version.txt')
                    }
                }
                stage('ARM') {
                    when { expression { return params.ARM } }
                    steps {
                        sh "echo ARM build will be running there"
                    }                        
                }
                stage('MacOS') {
                    when { expression { return params.MacOS } }                        
                    steps {
                        sh "MacOS build will be running there"
                    }
                }
            }
        }
        stage('SonarQube') {
            when { expression { params.BUILD_TYPE == 'Release' } }
            steps {
                sh """
                    if [ -n ${SONAR_TOKEN} ] && \
                      [ -n ${BUILD_TAG} ] && \
                      [ -n ${BRANCH_NAME} ]; then
                      sonar-scanner \
                        -Dsonar.login=${SONAR_TOKEN} \
                        -Dsonar.projectVersion=${BUILD_TAG} \
                        -Dsonar.branch=${BRANCH_NAME}
                    else
                      echo 'required env vars not found'
                    fi
                """
            }
        }
    }
    post {
        always {
            script {
                sh """
                  docker stop $IROHA_POSTGRES_HOST $IROHA_REDIS_HOST
                  docker rm $IROHA_POSTGRES_HOST $IROHA_REDIS_HOST
                  docker network rm $IROHA_NETWORK
                """
            }
        }
    }
}
