#!/bin/bash

set -euo pipefail

(( $# == 0 )) || exit 0

bim-client-config-update.sh most_recent_version 500
