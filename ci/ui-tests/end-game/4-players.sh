#!/bin/bash
# UI_TEST

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

"$script_dir"/../launch-test.sh \
             "$@" \
             --working-directory 4-players \
             "$script_dir"/4-players-game-player-1-wins.json \
             "$script_dir"/4-players-game-player-2-loses.json \
             "$script_dir"/4-players-game-player-3-loses.json \
             "$script_dir"/4-players-game-player-4-loses.json
