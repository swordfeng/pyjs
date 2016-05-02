{
    "targets": [
        {
            "target_name": "pyjs-native",
            "sources": [
                "src/pyjs.cc",
                "src/pyjsobject.cc",
                "src/pyjstypeconv.cc"
            ],
            "include_dirs" : [
                "<!(node -e \"require('nan')\")"
            ],
            "cflags": [
                "<!(python-config --cflags)"
            ],
            "ldflags": [
                "<!(python-config --ldflags)"
            ]
        }
    ]
}
