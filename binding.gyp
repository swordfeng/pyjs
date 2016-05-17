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
            "include_dirs" : [
                "<!(node -e \"require('nan')\")"
            ],
            "conditions": [
                ["OS=='win'", {

                }, {
                    "cflags": [
                        "<!(python3-config --includes)"
                    ],
                    "ldflags": [
                        "<!(python3-config --ldflags)"
                    ]
                }]
            ]
        }
    ]
}
