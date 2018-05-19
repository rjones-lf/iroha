{
  'variables': {
    'iroha_lib_dir': '../../'
  },
  'targets': [
    {
      'target_name': 'shared_model',
      'type': 'none',
      'actions': [
        {
          'action_name': 'configure',
          'message': 'Generate CMake build configuration for shared_model...',
          'inputs': [
            '<(iroha_lib_dir)/bindings/CMakeLists.txt'
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/bindings/Makefile',
          ],
          'action': [
            'cmake', 
            '-H<(iroha_lib_dir)', 
            '-B<(SHARED_INTERMEDIATE_DIR)', 
            '-DSWIG_NODE=ON', 
            '-DENABLE_LIBS_PACKAGING=OFF',
            '-DCMAKE_POSITION_INDEPENDENT_CODE=ON',
            '-DCMAKE_BUILD_TYPE=Release'
          ],
        },
        {
          'action_name': 'build',
          'message': 'Build shared_model libraries by CMake...',
          'inputs': [
            '<(SHARED_INTERMEDIATE_DIR)/bindings/Makefile',
          ],
          'outputs': [
            '<(SHARED_INTERMEDIATE_DIR)/bindings/bindingsJAVASCRIPT_wrap.cxx',
            '<(SHARED_INTERMEDIATE_DIR)/bindings/libirohanode.a',
            '<(SHARED_INTERMEDIATE_DIR)/bindings/libbindings.a'
          ],
          'action': [
            'cmake', 
            '--build', '<(SHARED_INTERMEDIATE_DIR)',
            '--target', 'irohanode',
            '--',
            '-j<!(echo "$(getconf _NPROCESSORS_ONLN)")'
          ]
        },
      ],
      ###
      # Copy all necessary static libs to PRODUCT_DIR, so we ensure their existence!
      ###
      'copies': [
        {
          'files': [
            '<(SHARED_INTERMEDIATE_DIR)/bindings/libirohanode.a',
            '<(SHARED_INTERMEDIATE_DIR)/bindings/libbindings.a',
            '<(SHARED_INTERMEDIATE_DIR)/generator/libgenerator.a',
            '<(SHARED_INTERMEDIATE_DIR)/amount/libiroha_amount.a',
            '<(SHARED_INTERMEDIATE_DIR)/schema/libschema.a',
            '<(SHARED_INTERMEDIATE_DIR)/validators/libshared_model_stateless_validation.a',
            '<(SHARED_INTERMEDIATE_DIR)/cryptography/ed25519_sha3_impl/libshared_model_cryptography.a',
            '<(SHARED_INTERMEDIATE_DIR)/cryptography/ed25519_sha3_impl/internal/libhash.a',
            '<(SHARED_INTERMEDIATE_DIR)/cryptography/ed25519_sha3_impl/internal/libed25519_crypto.a',
            '<(SHARED_INTERMEDIATE_DIR)/cryptography/model_impl/libshared_model_cryptography_model.a',

            # Third-party libraries
            '<(iroha_lib_dir)/external/src/hyperledger_ed25519-build/libed25519.a'
          ],
          'destination': '<(PRODUCT_DIR)'
        }
      ]
    },
    {
      'target_name': '<(module_name)',
      'dependencies': [ 'shared_model' ],
      'include_dirs': [
        '<(iroha_lib_dir)',
        # TODO: Remove these include directories when Shared Model 
        # will be completely separated from Iroha
        '<(iroha_lib_dir)/../schema',
        '<(iroha_lib_dir)/../libs'
      ],
      'sources': [
        '<(SHARED_INTERMEDIATE_DIR)/bindings/bindingsJAVASCRIPT_wrap.cxx'
      ],
      'cflags_cc': ['-std=c++14', '-fexceptions'],
      'cflags_cc!': ['-fno-rtti'],
      'libraries': [
        '-L/usr/local/lib',
        '-L<(PRODUCT_DIR)',

        '-lirohanode', # Library contains SWIG runtime
        '-lbindings',
        '-lgenerator',
        '-liroha_amount',
        '-lschema',
        '-lshared_model_stateless_validation',
        # Cryptography libs
        '-lshared_model_cryptography',
        '-lhash',
        '-led25519_crypto',
        '-lshared_model_cryptography_model',

        # Third-party libraries
        '-led25519',
        '-lprotobuf'
      ],
      'conditions': [
        [ 'OS == "mac"', {
            'xcode_settings': {
              'GCC_ENABLE_CPP_RTTI': 'YES',
              'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
              'OTHER_CFLAGS': ['-std=c++14']
            }
          }
        ]
      ]
    },
    {
      'target_name': 'action_after_build',
      'type': 'none',
      'dependencies': [ '<(module_name)' ],
      'copies': [
        {
          'files': [ '<(PRODUCT_DIR)/<(module_name).node' ],
          'destination': '<(module_path)'
        }
      ]
    }
  ]
}
