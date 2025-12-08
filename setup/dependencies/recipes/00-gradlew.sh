#!/bin/bash

set -euo pipefail

: "${bim_packages_root:-}"
: "${bim_target_platform:-}"

# This script installs the Gradle Wrapper on the host, but only if the
# target is Android.
[[ "$bim_target_platform" == "android" ]] || exit 0

gradlew_version=8.13
package_revision=2
version="$gradlew_version"-"$package_revision"
build_type=release

! bim-install-package gradlew "$version" "$build_type" 2>/dev/null \
    || exit 0

install_dir="$bim_packages_root"/gradlew/install-"$build_type"
install_path="$install_dir"/var/gradlew/

rm --force --recursive "$install_dir"
mkdir --parents "$install_path"

cd "$install_path"

# Gradle thinks it is a good idea to consume stdin even though it is
# not needed, breaking outer pipes currently processing stdin. As a
# workaround we explicitly send nothing on the process' input.  See
# https://github.com/gradle/gradle/issues/14961
gradle wrapper --gradle-version="$gradlew_version" < /dev/null

rm gradlew.bat
mkdir "$install_dir"/bin

ln -s "$(relative-path "$install_dir"/bin ./gradlew)" "$install_dir"/bin/

bim-package-and-install "$install_dir" gradlew "$version" "$build_type"
