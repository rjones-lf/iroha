#!/usr/bin/env groovy

def previousCommitOrCurrent() {
	// GIT_PREVIOUS_COMMIT is null on first PR build
	// regardless Jenkins docs saying it equals the current one on first build in branch
	echo "!env.GIT_PREVIOUS_COMMIT ? env.GIT_COMMIT : env.GIT_PREVIOUS_COMMIT"
	try {
	  print "!${env.GIT_PREVIOUS_COMMIT} ? ${env.GIT_COMMIT }: ${env.GIT_PREVIOUS_COMMIT}"
  }catch(Exception e) {
    print "111111111111Error: " + e
  }
	return !env.GIT_PREVIOUS_COMMIT ? env.GIT_COMMIT : env.GIT_PREVIOUS_COMMIT
}

return this
