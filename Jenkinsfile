pipeline {
  environment {
    sorabot = credentials('jenkins-integration-test')
  }
  options {
    skipDefaultCheckout()
  }
  agent none
  stages {
    stage ('all') {
      parallel {
        stage ('checkout') {
          agent { label 'x86_64_aws_cov' }
          steps {
            script {
              if (env.CHANGE_ID) {
                checkout changelog: false, poll: false, scm: [$class: 'GitSCM', branches:
                  [[name: "${CHANGE_BRANCH}"]], doGenerateSubmoduleConfigurations: false, extensions:
                  [[$class: 'PreBuildMerge', options: [fastForwardMode: 'FF_ONLY', mergeRemote: 'origin',
                  mergeStrategy: 'default', mergeTarget: "${CHANGE_TARGET}"]], [$class: 'LocalBranch'],
                  [$class: 'CleanCheckout'], [$class: 'PruneStaleBranch'], [$class: 'UserIdentity',
                  email: 'jenkins@soramitsu.co.jp', name: 'jenkins']], submoduleCfg: [], userRemoteConfigs:
                  [[credentialsId: 'sorabot-github-user', url: 'https://github.com/hyperledger/iroha.git']]]
              }
              sh('echo This commit is mergeable')
            }
          }
        }
      }
      post {
        success {
          node('master') {
            script {
              if (env.CHANGE_ID) {
                waitUntil {
                  def approvalsRequired = 1
                  def mergeApproval = input message: "Your PR has been built successfully. Merge ${CHANGE_BRANCH} into ${CHANGE_TARGET}?"
                  def slurper = new groovy.json.JsonSlurperClassic()
                  def jsonResponseReview = sh(script: """
                    curl -H "Authorization: token ${sorabot}" -H "Accept: application/vnd.github.v3+json" https://api.github.com/repos/hyperledger/iroha/pulls/${CHANGE_ID}/reviews
                  """, returnStdout: true).trim()
                  jsonResponseReview = slurper.parseText(jsonResponseReview)
                  if (jsonResponseReview.size() > 0) {
                    jsonResponseReview.each {
                      if ("${it.state}" == "APPROVED") {
                        approvalsRequired -= 1
                      }
                    }
                  }
                  if (approvalsRequired > 0) {
                    sh "echo 'Merge failed. Get more PR approvals before merging'"
                    return false
                  }
                  return true
                }
                def jsonMergeResult = sh(script: """
                  curl -X PUT -H "Authorization: token ${sorabot}" -H "Accept: application/vnd.github.v3+json" https://api.github.com/repos/hyperledger/iroha/pulls/${CHANGE_ID}/merge
                """)
                jsonMergeResult = slurper.parseText(jsonMergeResult)
                sh("echo ${jsonMergeResult.message}")
              }
            }
          }
        }
      }
    }
  }
}
