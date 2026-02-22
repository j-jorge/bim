#!/bin/bash
# UI_TEST

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

LANG=en \
    "$script_dir"/../launch-test.sh \
    "$@" \
    "$script_dir"/language-buttons-in-english.json
