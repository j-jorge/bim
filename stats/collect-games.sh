#!/bin/bash

set -euo pipefail

grep -h 'Sending game over' "$@" \
    | sed 's/^.\([0-9]\+-[0-9]\+-[0-9]\+\) \([0-9]\+\)\+.\+game_id=\([0-9]\+\).\+/\1-\2 \3/' \
    | uniq \
    | cut -d' ' -f1 \
    | uniq -c \
    | awk '{print $2, $1}'
