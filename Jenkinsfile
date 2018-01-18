// Overall pipeline looks like the following
//               |----Debug
//   |--x86------|----Release
//   | 
// --|    OR
//   |           
//   |--ARM------|----Debug
//               |----Release
//   |           
properties([parameters([
    choice(choices: 'Debug\nRelease', description: '', name: 'BUILD_TYPE'),
    booleanParam(defaultValue: true, description: '', name: 'x86'),
    booleanParam(defaultValue: false, description: '', name: 'ARM'),
    booleanParam(defaultValue: false, description: '', name: 'MacOS'),
    string(defaultValue: '8', description: 'How much parallelism should we exploit. "4" is optimal for machines with modest amount of memory and at least 4 cores', name: 'PARALLELISM')]),
    pipelineTriggers([cron('@weekly')])])
pipeline {
    environment {
        SORABOT_TOKEN = credentials('SORABOT_TOKEN')
        SONAR_TOKEN = credentials('SONAR_TOKEN')
        CODECOV_TOKEN = credentials('CODECOV_TOKEN')
        //IROHA_CONTAINER_NAME="iroha-`dd if=/dev/urandom bs=20 count=1 2> /dev/null| md5sum | cut -c1-8`"
        //IROHA_CONTAINER_NAME="iroha10"
        IROHA_NETWORK="iroha-network-1"
        //IROHA_NETWORK="iroha-`dd if=/dev/urandom bs=20 count=1 2> /dev/null| md5sum | cut -c1-8`"
        POSTGRES_NAME="pg-`dd if=/dev/urandom bs=20 count=1 2> /dev/null| md5sum | cut -c1-8`"
        REDIS_NAME="redis-`dd if=/dev/urandom bs=20 count=1 2> /dev/null| md5sum | cut -c1-8`"
        POSTGRES_USER="pg-user-`dd if=/dev/urandom bs=20 count=1 2> /dev/null| md5sum | cut -c1-8`"
        POSTGRES_PASSWORD="`dd if=/dev/urandom bs=20 count=1 2> /dev/null| md5sum | cut -c1-8`"
        POSTGRES_PORT=5432
        REDIS_PORT=6379
    }
    agent {
        label 'docker_1'
    }
    stages {
        stage('Build Debug') {
            when { expression { params.BUILD_TYPE == 'Debug' } }
            parallel {
                stage ('x86') {
                    when { expression { return params.x86 } }
                    steps {
                        script {
                            def p_c = docker.image('postgres:9.5').run("-e POSTGRES_USER=${env.POSTGRES_USER} -e POSTGRES_PASSWORD=${env.POSTGRES_PASSWORD} --name ${env.POSTGRES_NAME} --network=${env.IROHA_NETWORK}")
                            def r_c = docker.image('redis:3.2.8').run("--name ${env.REDIS_NAME} --network=${env.IROHA_NETWORK}")
                        
                            sh """
                                echo `docker inspect -f '{{range \$_, \$e := .Config.Env}}{{println \$e}}{{end}}' ${p_c.id} | grep POSTGRES_USER|cut -d '=' -f2` > pg-user
                                echo `docker inspect -f '{{range \$_, \$e := .Config.Env}}{{println \$e}}{{end}}' ${p_c.id} | grep POSTGRES_PASSWORD|cut -d '=' -f2` > pg-pass
                                echo `docker inspect -f '{{ .Name }}' ${p_c.id} | tr -d '/'` > pg-host
                                echo `docker inspect -f '{{ .Name }}' ${r_c.id} | tr -d '/'` > redis-host
                            """
                            IROHA_POSTGRES_HOST = readFile('pg-host').trim()
                            IROHA_POSTGRES_USER = readFile('pg-user').trim()
                            IROHA_POSTGRES_PASSWORD = readFile('pg-pass').trim()
                            IROHA_REDIS_HOST = readFile('redis-host').trim()
                            docker.image('hyperledger/iroha-docker-develop:v1').inside("-e IROHA_POSTGRES_HOST=${IROHA_POSTGRES_HOST} -e IROHA_POSTGRES_PORT=5432 -e IROHA_POSTGRES_USER=${IROHA_POSTGRES_USER} -e IROHA_POSTGRES_PASSWORD=${IROHA_POSTGRES_PASSWORD} -e IROHA_REDIS_HOST=${IROHA_REDIS_HOST} -e IROHA_REDIS_PORT=6379 --network=${env.IROHA_NETWORK}") {
                                def scmVars = checkout scm
                                env.IROHA_VERSION = "0x${scmVars.GIT_COMMIT}"
                                sh """
                                    ccache --version
                                    ccache --show-stats
                                    ccache --zero-stats
                                    ccache --max-size=1G
                                """
                                sh """
                                    cmake \
                                      -DCOVERAGE=ON \
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

                                // Codecov
                                sh "bash <(curl -s https://codecov.io/bash) -f build/reports/gcovr.xml -t ${CODECOV_TOKEN} || echo 'Codecov did not collect coverage reports'"

                                // Sonar
                                if (env.CHANGE_ID != null) {
                                    sh """
                                        sonar-scanner \
                                            -Dsonar.github.disableInlineComments \
                                            -Dsonar.github.repository='bakhtin/iroha' \
                                            -Dsonar.analysis.mode=preview \
                                            -Dsonar.login=${SONAR_TOKEN} \
                                            -Dsonar.projectVersion=${BUILD_TAG} \
                                            -Dsonar.github.oauth=${SORABOT_TOKEN} \
                                            -Dsonar.github.pullRequest=${CHANGE_ID}
                                    """ 
                                }

                                stash(allowEmpty: true, includes: 'build/compile_commands.json', name: 'Compile commands')
                                stash(allowEmpty: true, includes: 'build/reports/', name: 'Reports')
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
        stage('Code Quality') {
            when { expression { params.BUILD_TYPE == 'Debug' } }
            parallel {
                stage ('Tests') {
                    steps {
                        sh "echo Stub"
                    }
                }
                stage('SonarQube PR') {
                    when { 
                        expression { return env.CHANGE_ID != null }
                    }
                    steps {
                        sh "echo Stub Sonar"
                    }
                }
                stage('Codecov') {
                    steps {
                        sh "echo Stub codecov"
                    }
                }
            }
        }
        stage('Build Release') {
            when { expression { params.BUILD_TYPE == 'Release' } }
            parallel {
                stage('x86') {
                    when { expression { return params.x86 } }
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
                IROHA_POSTGRES_HOST = readFile('pg-host').trim()
                IROHA_REDIS_HOST = readFile('redis-host').trim()
                sh """
                  docker stop $IROHA_POSTGRES_HOST $IROHA_REDIS_HOST
                  docker rm $IROHA_POSTGRES_HOST $IROHA_REDIS_HOST
                  #docker network rm $IROHA_NETWORK
                """
            }
        }
    }
}