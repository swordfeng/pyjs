{
    "targets": [
        {
            "target_name": "pyjs-native",
            "sources": [
                "src/pyjs.cc",
                "src/pyjsobject.cc",
                "src/pyjstypeconv.cc",
                "src/py-gil-lock.cc"
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
