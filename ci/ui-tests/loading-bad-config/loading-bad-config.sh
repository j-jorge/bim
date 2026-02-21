#!/bin/bash
# UI_TEST

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

"$script_dir"/../launch-test.sh \
             "$@" \
             "$script_dir"/loading-bad-config.json \
             -- \
             --http-mockup "$script_dir"/http-mockup.json
