#!/bin/bash
# UI_TEST

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

"$script_dir"/../launch-test.sh \
             "$@" \
             --business-setup "$script_dir"/business-setup.sh \
             "$script_dir"/loading-version-update.json
