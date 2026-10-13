#!/bin/bash

set -euo pipefail

(( $# == 0 )) || exit 0

bim-client-config-update.sh most_recent_version '"abc"'
bim-game-feature-update.sh falling_blocks '"aaa"'
bim-game-feature-slots-update.sh 1 '"bbb"'
bim-shop-update.sh coins_small '"aaa"'

