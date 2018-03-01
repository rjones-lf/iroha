{
  "targets": [
    {
      "target_name": "irohanodejs",
      "include_dirs": [
        "../../shared_model",
        "../../libs",
        "../../external/src/martinmoene_optional/include",
        "../../irohad",
        "../../schema"
      ],
      "sources": [
        "./build_cmake/shared_model/bindings/bindingsJAVASCRIPT_wrap.cxx"
      ],
      "cflags_cc": ["-std=c++14", "-fexceptions", "-DDISABLE_BACKWARD"],
      "cflags_cc!": ["-fno-rtti"],
      "conditions": [
        [
          "OS != 'mac'",
          {
            "libraries": [
              "<(module_root_dir)/build_cmake/shared_model/bindings/libbindings.a",
              "<(module_root_dir)/build_cmake/shared_model/bindings/irohanode.so"
            ]
          }
        ],
        [
          "OS == 'mac'",
          {
            "libraries": [
              "<(module_root_dir)/build_cmake/shared_model/bindings/libbindings.a",
              "<(module_root_dir)/build_cmake/shared_model/bindings/irohanode.dylib"
            ],
            "xcode_settings": {
              "GCC_ENABLE_CPP_RTTI": "YES",
              "GCC_ENABLE_CPP_EXCEPTIONS": "YES",
              "OTHER_CFLAGS": ["-std=c++14", "-DDISABLE_BACKWARD"]
            }
          }
        ]
      ]
    }
  ]
}
