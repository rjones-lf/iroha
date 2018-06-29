#!/usr/bin/env groovy

def setParallelism(defaultParameter) {
  if (!defaultParameter) {
    return 4
  }
  if (env.NODE_NAME.contains('arm7')) {
    return 1
  }
  if (env.NODE_NAME.contains('mac')) {
    return 4
  }
  if (env.NODE_NAME.contains('x86_64')) {
    return 8
  }
  return defaultParameter
}

return this
