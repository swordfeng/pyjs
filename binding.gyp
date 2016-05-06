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
            "cflags": [
                "<!(python3-config --cflags | sed 's/-Wstrict-prototypes//')"
            ],
            "ldflags": [
                "<!(python3-config --ldflags)"
            ]
        }
    ]
}
