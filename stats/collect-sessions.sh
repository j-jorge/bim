#!/bin/bash

set -euo pipefail

grep -h 'Attach session [0-9]\+' "$@" \
    | sed 's/^.\([0-9]\+-[0-9]\+-[0-9]\+\) \([0-9]\+\)\+.\+Attach session \([0-9]\+\).\+/\1-\2 \3/' \
    | uniq \
    | cut -d' ' -f1 \
    | uniq -c \
    | awk '{print $2, $1}'
