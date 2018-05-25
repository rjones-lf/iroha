#!/usr/bin/env groovy

def notifyBuildResults() {
	def mergeMessage = ''
	def content = """ 
<h4>This email informs you about the build results on Jenkins CI</h4>
<h4>Build status: ${currentBuild.currentResult}. ${mergeMessage}</h4>
<p>
Check <a href = "${BUILD_URL}">console output</a> to view the results. 
</p>
<p>
You can find the build log attached to this email
</p>
	"""
	// notify commiter in case of branch commit
	if ( env.CHANGE_ID == null ) {
		def receivers = "${GIT_COMMITER_EMAIL}"
		sendEmail(content, receivers)
		return
	}
	// merge commit build results
	if ( params.Merge_PR ) {
		if ( currentBuild.currentResult == "SUCCESS" ) {
			mergeMessage = "Merged to ${env.CHANGE_TARGET}: true"
		}
		else {
			mergeMessage = "Merged to ${env.CHANGE_TARGET}: false"
		}

		if ( env.CHANGE_TARGET == 'master' ) {
			def receivers = "iroha-maintainers@soramitsu.co.jp"
			sendEmail(content, receivers)
		}
		elif ( env.CHANGE_TARGET == 'develop' ) {
			def receivers = "andrei@soramitsu.co.jp, fyodor@soramitsu.co.jp, ${GIT_COMMITER_EMAIL}"
			sendEmail(content, receivers)
		}
		elif ( env.CHANGE_TARGET == 'trunk' ) {
			def receivers = "${GIT_COMMITER_EMAIL}"
			sendEmail(content, receivers)
		}
	}
	else {
		// write comment if it is a pull request commit
		def notify = load ".jenkinsci/github-api.groovy"
		notify.writePullRequestComment()
	}
  return
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