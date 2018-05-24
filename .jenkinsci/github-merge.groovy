#!/usr/bin/env groovy

//@Grab(group='org.codehaus.groovy.modules.http-builder', module='http-builder', version='0.6.0')
//import groovyx.net.http.*


def mergePullRequest() {
  if ( !checkMergeAcceptance() ) {
  	return false
  }
	// TODO: fill commit message + commit title
	withCredentials([string(credentialsId: 'jenkins-integration-test', variable: 'sorabot')]) {
		def slurper = new groovy.json.JsonSlurperClassic()
		def commitTitle = "sample title"
		def commitMessage = "sample message"
		def mergeMethod = getMergeMethod()
		
		def jsonResponseReview = sh(script: """
		curl -H "Authorization: token ${sorabot}" \
				 -H "Accept: application/vnd.github.v3+json" \
				 -X PUT --data '{"commit_title":"${commitTitle}","commit_message":"${commitMessage}","sha":"${env.GIT_COMMIT}","merge_method":"${mergeMethod}"}' \
				 -w "%{http_code}" https://api.github.com/repos/hyperledger/iroha/pulls/${CHANGE_ID}/merge""", returnStdout: true)
		
		// def githubResponce = sh(script: """echo ${jsonResponseReview} | grep -E "^\\d{3}")""", returnStdout: true).trim()
		// jsonResponseReview = sh(script: """echo ${jsonResponseReview} | grep -v -E "^\\d{3}")""", returnStdout: true).trim()
		
		echo jsonResponseReview
		

		// if ( githubResponce != "200" ) {
		// 	return false
		// }
		jsonResponseReview = slurper.parseText(jsonResponseReview)
		if (jsonResponseReview.merge != "true") {
			return false
		}
	}
	return true
	
	// def github = new HTTPBuilder( 'https://api.github.com/repos/hyperledger/iroha/pulls/${CHANGE_ID}/merge' )

	// http.request (POST, JSON){ req ->
	// 	body = [
	// 		"commit_title" : "${commitTitle}",
	// 		"commit_message" : "${commitMessage}",
	// 		"sha" : "${env.GIT_COMMIT}",
	// 		"merge_method" : "${mergeMethod}"
	// 	]

	// 	headers.'Authorization' = "token ${sorabot}"
	// 	headers.'Accept' = "application/vnd.github.v3+json"

	// 	// success response handler
	// 	response.success = { resp, json ->
	// 		println ${json.message}
	// 		if ( ${json.merged} == 'true' ) { return true }
	// 		else { return false }
	// 	}

	// 	response.failure = { resp, json ->
	// 		println ${json.message}
	// 		println "Unexpected error: ${resp.statusLine.statusCode}"
	// 		return false
	// 	}
	// }
}

def checkMergeAcceptance() {
	def approvalsRequired = 2
	def gitCommitterEmail = sh(script: 'git --no-pager show -s --format=\'%ae\'', returnStdout: true).trim()
	wrap([$class: 'BuildUser']) {
		jenkinsCommitterEmail = env.BUILD_USER_EMAIL
  }
  withCredentials([string(credentialsId: 'jenkins-integration-test', variable: 'sorabot')]) {
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
		sh "echo approvals: ${approvalsRequired}"
		// if (approvalsRequired > 0) {
		//   sh "echo 'Merge failed. Get more PR approvals before merging'"
		//   return false
		// }
		// elif (gitCommitterEmail != jenkinsCommitterEmail) {
		//   sh "echo 'Merge failed. Email of the commit does not match Jenkins user'"
		//   return false
		// }
  }
  return true
}

def getMergeMethod() {
	if ( env.CHANGE_TARGET == 'master') {
		return "merge"
	}
	else {
		return "squash"
	}
}


return this
