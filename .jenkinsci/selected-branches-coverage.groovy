#!/usr/bin/env groovy

def selectedBranchesCoverage(branches, PRCoverage=true) {
	// trigger coverage if branch is either develop or master, or it is a PR
	if (PRCoverage) {
		return env.GIT_LOCAL_BRANCH in branches || env.CHANGE_ID != null
	}
	else {
		return env.GIT_LOCAL_BRANCH in branches
	}
}

return this