#!/usr/bin/env groovy

// replaces bunch of expressions in `when` block at the `stage` in Jenkinsfile
def prepare() {
	def mergeBranches = env.CHANGE_TARGET ==~ /(master|develop|trunk)/ ? "true" : "false"
	def docsPullRequestBranches = env.CHANGE_TARGET ==~ /(master|develop)/ ? "true" : "false"
  def pullRequestCommit = env.CHANGE_ID && env.GIT_PREVIOUS_COMMIT ? "true" : "false"
  INITIAL_COMMIT_PR = env.CHANGE_ID && env.GIT_PREVIOUS_COMMIT == null ? "true" : "false"
  MERGE_CONDITIONS_SATISFIED = (mergeBranches == "true" && pullRequestCommit == "true" && params.merge_pr) ? "true" : "false"
  REST_PR_CONDITIONS_SATISFIED = (docsPullRequestBranches == "true" && pullRequestCommit == "true" && params.merge_pr) ? "true" : "false"
  GIT_COMMITER_EMAIL = sh(script: """git --no-pager show -s --format='%ae' ${env.GIT_COMMIT}""", returnStdout: true).trim()
}

return this
