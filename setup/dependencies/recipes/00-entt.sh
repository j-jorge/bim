#!/bin/bash

set -euo pipefail

: "${entt_repository:=https://github.com/skypjack/entt/}"
: "${entt_version:=3.14.0}"
package_revision=3
package_version="$entt_version"-"$package_revision"
build_type=release

! bim-install-package entt "$package_version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh entt "$build_type"

bim-git-clone-repository "$entt_repository" "v$entt_version" "$source_dir"

bim-cmake-build \
    --build-dir "$build_dir" \
    --build-type "${build_type^}" \
    --install-dir "$install_dir" \
    --source-dir "$source_dir"

bim-package-and-install "$install_dir" entt "$package_version" "$build_type"
