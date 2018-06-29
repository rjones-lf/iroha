#!/usr/bin/env groovy

def notifyBuildResults() {
	def mergeMessage = ''
	def receivers = ''
	// notify commiter in case of branch commit
	if ( env.CHANGE_ID == null ) { 
		sendEmail(buildContent(mergeMessage), "${GIT_COMMITER_EMAIL}")
		return
	}
	// merge commit build results
	if ( params.merge_pr ) {
		if ( currentBuild.currentResult == "SUCCESS" ) {
			mergeMessage = "Merge status to ${env.CHANGE_TARGET}: true"
		}
		else {
			mergeMessage = "Merge status to ${env.CHANGE_TARGET}: false"
		}

		if ( env.CHANGE_TARGET == 'master' ) {
			receivers = "iroha-maintainers@soramitsu.co.jp"
		}
		else if ( env.CHANGE_TARGET == 'develop' ) { 
			receivers = "andrei@soramitsu.co.jp, fyodor@soramitsu.co.jp, ${GIT_COMMITER_EMAIL}"
		}
		else {
			receivers = "${GIT_COMMITER_EMAIL}"
		}

		sendEmail(buildContent(mergeMessage), receivers)
	}
	else {
		// write comment to the PR page on github if it is a pull request commit
		def notify = load ".jenkinsci/github-api.groovy"
		notify.writePullRequestComment()
	}
  return
}

def buildContent(mergeMessage="") {
	return """
<h4>This email informs you about the build results on Jenkins CI</h4>
<h4>Build status: ${currentBuild.currentResult}. ${mergeMessage}</h4>
<p>
Check <a href = "${BUILD_URL}">console output</a> to view the results. 
</p>
<p>
You can find the build log attached to this email
</p>
"""
}

def sendEmail(content, to) {
	emailext( subject: '$DEFAULT_SUBJECT',
            body: "${content}",
            attachLog: true,
            compressLog: true,
            to: "${to}"
  )
}
return this
