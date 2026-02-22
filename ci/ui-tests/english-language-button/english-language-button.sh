#!/bin/bash
# UI_TEST

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

LANG=fr \
    "$script_dir"/../launch-test.sh \
    "$@" \
    "$script_dir"/english-language-button.json
