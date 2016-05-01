{
    "targets": [
        {
            "target_name": "pyjs-native",
            "sources": [ "pyjs.cc" ],
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
