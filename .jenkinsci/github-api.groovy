#!/usr/bin/env groovy

def mergePullRequest() {
  if ( !checkMergeAcceptance() ) {
  	return false
  }
	withCredentials([string(credentialsId: 'jenkins-integration-test', variable: 'sorabot')]) {
		def slurper = new groovy.json.JsonSlurperClassic()
		def commitTitle = ""
		def commitMessage = ""
		def mergeMethod = getMergeMethod()
		
		def jsonResponseMerge = sh(script: """
		curl -H "Authorization: token ${sorabot}" \
				 -H "Accept: application/vnd.github.v3+json" \
				 -X PUT --data '{"commit_title":"${commitTitle}","commit_message":"${commitMessage}","sha":"${env.GIT_COMMIT}","merge_method":"${mergeMethod}"}' \
				 -w "%{http_code}" https://api.github.com/repos/hyperledger/iroha/pulls/${CHANGE_ID}/merge""", returnStdout: true)
		
		def githubResponce = sh(script:'printf ${jsonResponseMerge} | grep -E "\\d{3}', returnStdout: true).trim()
		jsonResponseMerge = sh(script:'printf ${jsonResponseMerge} | grep -v -E "\\d{3}', returnStdout: true).trim()
		
		if ( githubResponce != "200" ) {
			return false
		}
		jsonResponseMerge = slurper.parseText(jsonResponseMerge)
		if (jsonResponseMerge.merged != "true") {
			return false
		}
	}
	return true
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
		if (approvalsRequired > 0) {
		  sh "echo 'Merge failed. Get more PR approvals before merging'"
		  return false
		}
		elif (gitCommitterEmail != jenkinsCommitterEmail) {
		  sh "echo 'Merge failed. Email of the commit does not match Jenkins user'"
		  return false
		}
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

def getPullRequestReviewers() {
	def slurper = new groovy.json.JsonSlurperClassic()
	def ghUsersList = ''
	// there is no github API that allows to get all reviewers for the PR. 
	// thus we use github API twice to request for reviews and requested reviewers. afterwards, results are concatenated with @
	def jsonResponseReviewers = sh(script: """
		curl https://api.github.com/repos/hyperledger/iroha/pulls/${CHANGE_ID}/requested_reviewers
		""", returnStdout: true).trim()
	def jsonResponseReview = sh(script: """
		curl https://api.github.com/repos/hyperledger/iroha/pulls/${CHANGE_ID}/reviews
		""", returnStdout: true).trim()

	// echo jsonResponseReview
	// echo jsonResponseReviewers
	jsonResponseReviewers = slurper.parseText(jsonResponseReviewers)
	if (jsonResponseReviewers.size() > 0) {
	  jsonResponseReviewers.users.each {
	  	ghUsersList = ghUsersList.concat("@${it.login} ")
	  }
	}
	jsonResponseReview = slurper.parseText(jsonResponseReview)
	if (jsonResponseReview.size() > 0) {
	  jsonResponseReview.each {
	  	if ("${it.state}" == "APPROVED") {
	  		ghUsersList = ghUsersList.concat("@${it.user.login} ")
	  	}
	  }
	}
	return ghUsersList
}

// comment to github issues with reviewers mentions with build status
def writePullRequestComment() {
	def ghUsersList = getPullRequestReviewers()
	withCredentials([string(credentialsId: 'jenkins-integration-test', variable: 'sorabot')]) {
		def slurper = new groovy.json.JsonSlurperClassic()
		def jsonResponseComment = sh(script: """
			curl -H "Authorization: token ${sorabot}" \
			-H "Accept: application/vnd.github.v3+json" \
			-X POST --data '{"body":"${ghUsersList} commit ${env.GIT_COMMIT} build status: ${currentBuild.currentResult}"}' \
			-w "%{http_code}" https://api.github.com/repos/hyperledger/iroha/issues/${CHANGE_ID}/comments
			""", returnStdout: true).trim()
		def githubResponce = sh(script:'printf ${jsonResponseComment} | grep -E "\\d{3}"', returnStdout: true).trim()
		if (githubResponce == "201") {
			return true
		}
	}
	return false
}

return this
