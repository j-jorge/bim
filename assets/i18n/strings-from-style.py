#!/bin/env python3
import json
import typing
import sys


def collect_strings(output: typing.TextIO, document):
    if isinstance(document, list):
        for s in document:
            collect_strings(output, s)
    elif isinstance(document, dict):
        for k, v in document.items():
            if k.endswith(".i18n") and isinstance(v, str):
                output.write(f'ic_gettext("{v}");\n')
            else:
                collect_strings(output, v)


if len(sys.argv) < 2:
    print(f"Usage: {sys.argv[0]} ouptut files…")
    sys.exit(1)

with open(sys.argv[1], "w") as output:
    for style_file in sys.argv[2:]:
        with open(style_file) as json_file:
            d = json.load(json_file)
            if isinstance(d, dict):
                collect_strings(output, d)
