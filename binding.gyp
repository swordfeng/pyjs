{
    "targets": [
        {
            "target_name": "pyjs-native",
            "sources": [
                "src/pyjs.cc",
                "src/jsobject.cc",
                "src/typeconv.cc",
                "src/gil-lock.cc",
                "src/pyjsfunction.cc",
                "src/pymodule.cc",
                "src/error.cc"
            ],
            "include_dirs": [
                "<!(node -e \"require('nan')\")"
            ],
            "conditions": [
                ["OS=='win'", {
                    "libraries": [ "<!(python -c \"from distutils import sysconfig; print(sysconfig.get_config_var('prefix')+'\\\\\\\\libs\\\\\\\\python'+sysconfig.get_config_var('VERSION')+'.lib');\")" ],
                    "include_dirs": [ "<!(python -c \"from distutils import sysconfig; print(sysconfig.get_config_var('INCLUDEPY'));\")" ]
                }],
                ["OS=='mac'", {
                    "xcode_settings": {
                        "OTHER_CPLUSPLUSFLAGS": [
                            "<!(python3-config --includes)",
                            "-std=c++14",
                            "-stdlib=libc++"
                        ],
                        "OTHER_LDFLAGS": [
                            "<!(python3-config --ldflags)",
                            "-stdlib=libc++"
                        ]
                    }
                }],
                ["OS=='linux'", {
                    "cflags": [
                        "<!(python3-config --cflags | sed 's/-Wstrict-prototypes//')",
                        "-Waddress",
                        "-Wconversion"
                    ],
                    "ldflags": [
                        "<!(python3-config --ldflags)"
                    ],
                    "defines": [
                        "LINUX",
                        "PYTHON_LIB=\"<!(python-config --libs | sed 's/-l\\(python\\S*\\).*/lib\\1.so/')\""
                    ]
                }]
            ]
        }
    ]
}
