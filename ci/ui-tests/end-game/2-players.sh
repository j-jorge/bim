#!/bin/bash
# UI_TEST

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

"$script_dir"/../launch-test.sh \
             "$@" \
             --working-directory 2-players \
             "$script_dir"/2-players-game-player-1-wins.json \
             "$script_dir"/2-players-game-player-2-loses.json
