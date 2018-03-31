#!/usr/bin/env groovy

def selectedBranchesCoverage(branches, PRCoverage=true) {
	// trigger coverage if branch is either develop or master, or it is a PR
	if (PRCoverage) {
		return env.BRANCH_NAME in branches || env.CHANGE_ID != null
	}
	else {
		return env.BRANCH_NAME in branches	
	}
}

return this