#!/usr/bin/env groovy

// list of the pull requests reviews status on github
enum GithubPRStatus {
	APPROVED, CHANGES_REQUESTED, REVIEW_REQUESTED
}
// list of the possible merge strategies
enum MergeTarget {
	merge, squash
}
// list of supportable target branches for automated merge (by CI)
enum ChangeTarget {
	master, develop, trunk
}

// map with user:review_status
pullRequestReviewers = [:]

// merges pull request using GitHub API in case it meets the merging criteria
def mergePullRequest() {
	if ( ! ( checkMergeAcceptance() ) ) { return false  }
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
		def githubResponse = sh(script:"""set +x; printf '%s\n' "${jsonResponseMerge}" | tail -n 1; set -x""", returnStdout: true).trim()
		jsonResponseMerge = slurper.parseText(jsonResponseMerge)
		if (jsonResponseMerge.merged != "true" || !(githubResponse ==~ "200")) {
			echo jsonResponseMerge.message
			return false
		}
		return true
	}
}

// check merge acceptance by:
// - at least 2 "approved and NO "changes_requested" in reviews
// - e-mail of the commit does not match Jenkins user who launched this build
def checkMergeAcceptance() {
	def approvalsRequired = 0
	// fill the map of user:review_status
  getPullRequestReviewers()
  pullRequestReviewers.each{ user, review_status -> 
  	if (review_status == GithubPRStatus.APPROVED.toString()) {
  		approvalsRequired += 1
  	}
  	else if (review_status == GithubPRStatus.CHANGES_REQUESTED.toString()) {
  		return false
  	}
  }
	if (approvalsRequired < 2) {
		sh "echo 'Merge failed. Get more PR approvals before merging'"
		return false
	}
	return true
}

// returns merge method based on target branch (squash&merge vs merge)
def getMergeMethod() {
	if (env.CHANGE_TARGET == ChangeTarget.master.toString()) {
		return MergeTarget.merge.toString()
	}
	else {
		return MergeTarget.squash.toString()
	}
}

// fill the pullRequestReviews map with user:review status
def getPullRequestReviewers() {
	def slurper = new groovy.json.JsonSlurperClassic()
	// if there more than 1 page of "reviews" in PR (it happens due to huge amount of comments)
	def reviewPaging = sh(script: """curl -I https://api.github.com/repos/hyperledger/iroha/pulls/1392/reviews | grep -E "^Link:" | wc -l""", returnStdout: true)
	def reviewPagesCount = "1"
	if (reviewPaging.toInteger()) {
		reviewPagesCount = sh(script: """ curl -I https://api.github.com/repos/hyperledger/iroha/pulls/1392/reviews | grep -E "^Link:" | awk 'BEGIN { FS = "page" } { print \$NF }' | awk -F"=" '{print \$2}' | awk -F">" '{print \$1}' """, returnStdout: true)
	}
	// start the loop to request pages sequentially
	for(pageID in (1..reviewPagesCount.toInteger())) {
		def jsonResponseReview = sh(script: """
		curl https://api.github.com/repos/hyperledger/iroha/pulls/${CHANGE_ID}/reviews?page=${pageID}
		""", returnStdout: true).trim()
		// process returned reviews. add/update user:review_status to the map
		jsonResponseReview = slurper.parseText(jsonResponseReview)
		if (jsonResponseReview.size() > 0) {
			jsonResponseReview.each {
				if (it.state.toString() in [GithubPRStatus.APPROVED.toString(), GithubPRStatus.CHANGES_REQUESTED.toString()]) {
					pullRequestReviewers[it.user.login.toString()] = it.state.toString()
				}
			}
		}
	}
	// get requested reviewers (those who did not review this PR yet)
	def jsonResponseReviewers = sh(script: """
		curl https://api.github.com/repos/hyperledger/iroha/pulls/${CHANGE_ID}/requested_reviewers
		""", returnStdout: true).trim()
		jsonResponseReviewers = slurper.parseText(jsonResponseReviewers)
	if (jsonResponseReviewers.size() > 0) {
		jsonResponseReviewers.users.each {
			pullRequestReviewers[it.login] = GithubPRStatus.REVIEW_REQUESTED.toString()
		}
	}
}

// returns PR reviewers in the form of "@reviewer1 @reviewer2 ... @reviewerN" to mention PR reviewers about build result
def getUsersMentionList() {
	getPullRequestReviewers()
	def ghUsersList = ''
	pullRequestReviewers.each{ user, review_status -> ghUsersList = ["@${user}", ghUsersList].join(' ') }
	return ghUsersList
}

// post a comment on PR via GitHub API
def writePullRequestComment() {
	def ghUsersList = getUsersMentionList()
	withCredentials([string(credentialsId: 'jenkins-integration-test', variable: 'sorabot')]) {
		def slurper = new groovy.json.JsonSlurperClassic()
		def jsonResponseComment = sh(script: """
			curl -H "Authorization: token ${sorabot}" \
			-H "Accept: application/vnd.github.v3+json" \
			-X POST --data '{"body":"${ghUsersList} commit ${env.GIT_COMMIT} build status: ${currentBuild.currentResult}. build URL: ${BUILD_URL}"}' \
			-w "%{http_code}" https://api.github.com/repos/hyperledger/iroha/issues/${CHANGE_ID}/comments
			""", returnStdout: true).trim()		
		def githubResponse = sh(script:"""set +x; printf '%s\n' "${jsonResponseComment}" | tail -n 1 ; set -x""", returnStdout: true).trim()
		if (githubResponse ==~ "201") {
			return true
		}
	}
	return false
}

return this
