#!/bin/bash

set -euo pipefail

echo "Disk usage -- Start up."
df -hl

to_remove=(
    # Android tools.
    /usr/local/lib/android
    # DotNet
    /usr/share/dotnet
    # Haskell
    /opt/ghc
    /usr/local/.ghcup
    # Google tools
    /usr/lib/google-cloud-sdk
)

rm --force --recursive "${to_remove[@]}"

echo "Disk usage -- Removed directories and tools."
df -hl

rm --force --recursive "${RUNNER_TOOL_CACHE:?}"/*

echo "Disk usage -- Removed runner tool cache."
df -hl
