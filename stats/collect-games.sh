#!/bin/bash

set -euo pipefail

grep -h 'Creating new game' "$@" \
    | sed 's/^.\([0-9]\+-[0-9]\+-[0-9]\+\) \([0-9]\+\)\+.\+/\1-\2/' \
    | uniq -c \
    | awk '{print $2, $1}'
