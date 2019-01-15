###List of parameters for Jenkins "Custom command" option:

```bash
<optin_name> = <default_value> [(or <second_default_value> if <git branch name>)] - <descriptions> Ex: <Example of Use>
```

-  `x64linux_compiler_list = ['gcc5']` - Linux compiler name to build, Ex: `x64linux_compiler_list = ['gcc5','gcc7', 'clang6' , 'clang7']`

-  `mac_compiler_list = []` - Mac compiler name to build, Ex: `mac_compiler_list = ['appleclang']`

-  `testing = true` - Run test for each selected compiler, in jenkins will be several reports
  
-  `testList = '(module)'` - Test Regex name, Ex: `testList = '()'`-All,  `testList = '(module|integration|system|cmake|regression|benchmark|framework)'`

-  `sanitize = false` -  Adds cmakeOptions `-DSANITIZE='address;leak'` Ex: `sanitize=true;`

-  `cppcheck = false` - Runs `cppcheck` Ex: `cppcheck = true`

-  `fuzzing = false`  - builds fuzzing tests, work only with `x64linux_compiler_list = ['clang6']` Ex: `x64linux_compiler_list= ['clang6'];  testing = true; testList = "(None)"`

-  `sonar = false` - Runs Sonar Analysis, runs only on Linux Ex: `sonar = false;x64linux_compiler_list= ['gcc5','gcc7']`

-  `coverage = false` - Runs coverage, will run only if `testing = true`  Ex: `coverage = true`

-  `doxygen = false (or = true if mater|develop|dev )` Build doxygen, if specialBranch== true will publish, if not specialBranch will upload it to jenkins,  Ex: `doxygen=true`

-  `build_type = 'Debug'` - Sets `-DCMAKE_BUILD_TYPE=...` Ex: `build_type = 'Release';packageBuild = true;testing=false`

-  `packageBuild = false`  - Build package Work only with `build_type = 'Release'` and  `testing=false` Ex: `packageBuild = ture;build_type = 'Release';testing=false`

-  `pushDockerTag = 'not-supposed-to-be-pushed'(or = latest if mater, or = develop if develop|dev)` - if `packagePush=true` it the name of docker tag that will be pushed Ex: `packageBuild = true;build_type = 'Release';testing=false;packagePush=true`

-  `packagePush = false (or = true if mater|develop|dev )` - push all packages and docker to the artifactory and docker hub Ex:`packagePush=true;packageBuild = true;build_type = 'Release';testing=false`

-  `specialBranch = false (or = true if mater|develop|dev )`, Not recommended to set, it used to decide push `doxygen` and `iroha:develop-build` or not, and force to run `build_type = 'Release'` 
