#!/usr/bin/env groovy

def checkCoverageConditions() {
	// trigger coverage if branch is master, or it is a open PR commit, or a merge request
	def branch_coverage = ['master']

	if ( params.coverage ) {
		return true
	}
	else {
		return env.GIT_LOCAL_BRANCH in branch_coverage || INITIAL_COMMIT_PR == "true" || MERGE_CONDITIONS_SATISFIED == "true"
	}
}

return this
