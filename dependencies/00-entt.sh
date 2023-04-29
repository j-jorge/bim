#!/bin/bash

set -euo pipefail

: "${entt_repository:=https://github.com/skypjack/entt/}"
: "${entt_commit:=v3.11.1}"
package_revision=1
version="$entt_commit"-"$package_revision"
build_type=release

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"
. "$script_dir"/packaging.sh

! install_package entt "$version" "$build_type" 2>/dev/null \
    || exit 0

source_dir="$bomb_packages_root"/entt/source
build_dir="$bomb_packages_root"/entt/build-"$build_type"
install_dir="$bomb_packages_root"/entt/install-"$build_type"

mkdir --parents "$source_dir" "$build_dir" "$install_dir"

git_clone_repository "$entt_repository" "$entt_commit" "$source_dir"

cd "$build_dir"
cmake "$source_dir" \
      -DCMAKE_BUILD_TYPE="${build_type^}" \
      -DCMAKE_INSTALL_PREFIX="$install_dir"
cmake --build . --target install --parallel

package_and_install "$install_dir" entt "$version" "$build_type"
