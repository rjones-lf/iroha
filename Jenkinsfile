pipeline {
  agent { label 'master' }
  stages {
    stage ('XUnit') {
      steps {
        script {
          sh "mkdir reports"
          sh "cd reports && wget https://s3-eu-west-1.amazonaws.com/soramitsu-xunit-test/1.xml https://s3-eu-west-1.amazonaws.com/soramitsu-xunit-test/2.xml"
          docker.image('python:2.7').inside {
            sh 'python .jenkinsci/platform_tag.py "Linux \$(uname -m)" reports/1.xml'
            sh 'python .jenkinsci/platform_tag.py "Darwin \$(uname -m)" reports/2.xml'
            xunit testTimeMargin: '3000', thresholdMode: 2, thresholds: [failed(failureNewThreshold: '90', \
            failureThreshold: '50', unstableNewThreshold: '50', unstableThreshold: '20'), \
              skipped()], tools: [CTest(deleteOutputFiles: false, failIfNotNew: false, \
              pattern: 'reports/*.xml', skipNoTestFiles: false, stopProcessingIfError: true)]
          }
        }
      }
      post {
        always {
          cleanWs()
        }
      }
    }
  }
}
