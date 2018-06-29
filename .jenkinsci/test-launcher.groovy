#!/usr/bin/env groovy

// format the enum elements output like "(val1|val2|...|valN)*"
def printRange(start, end) {
  def output = ""
  def set = start..end
  TestTypes.values().each { t -> 
  	if (t.getOrder() in set) {
  		output = [output, (t.getOrder() != start ? "|" : ""), t.name()].join('')
  	}
  }
  return ["(", output, ")*"].join('')
}

// return tests list regex that will be launched by ctest
def chooseTestType() {
	if (params.merge_pr) {
		if (env.NODE_NAME.contains('x86_64')) {
			// choose module, integration, system, cmake, regression tests
			return printRange(TestTypes.module.getOrder(), TestTypes.regression.getOrder())
		}
		else {
			// not to do any tests
			return ""
		}
	}
	if (params.nightly) {
		if (env.NODE_NAME.contains('x86_64')) {
			// choose all tests
			return printRange(TestTypes.MIN_VALUE.getOrder(), TestTypes.MAX_VALUE.getOrder())
		}
		else {
			// choose module, integration, system, cmake, regression tests
			return printRange(TestTypes.module.getOrder(), TestTypes.regression.getOrder())
		}
	}
	// just choose module tests
	return [TestTypes.module.toString(), "*"].join('')
}

return this
