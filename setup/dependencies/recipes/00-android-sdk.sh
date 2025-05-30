#!/bin/bash

set -euo pipefail

: "${bim_packages_root:-}"
: "${bim_target_platform:-}"

# This script installs the Android SDK on the host, but only if the
# target is Android.
[[ "$bim_target_platform" == "android" ]] || exit 0

sdk_version=11076708
package_revision=3
version="$sdk_version"-"$package_revision"
build_type=release

! bim-install-package android-sdk "$version" "$build_type" 2>/dev/null \
    || exit 0

archive_name=commandlinetools-linux-"$sdk_version"_latest.zip
archive_url="https://dl.google.com/android/repository/${archive_name}"

source_dir="$bim_packages_root"/android-sdk/source
install_dir="$bim_packages_root"/android-sdk/install-"$build_type"

mkdir --parents "$source_dir"

cd "$source_dir"

download \
    --url="$archive_url" \
    --target-file="$archive_name" \
    --mime-type=application/zip \
    --skip-size-test

rm --force --recursive cmdline-tools
sdk_dir="$(bim-android-config --prefix "$install_dir" --sdk-root)"
rm --force --recursive "$sdk_dir"
mkdir --parents "$sdk_dir"/cmdline-tools/

unzip -x "$archive_name"
mv cmdline-tools "$sdk_dir"/cmdline-tools/latest

sdk_manager="$sdk_dir"/cmdline-tools/latest/bin/sdkmanager

yes | "$sdk_manager" --licenses > /dev/null || true
yes | "$sdk_manager" \
          "build-tools;35.0.0" \
          "ndk;28.1.13356709" \
          "platforms;android-21" \
          "platforms;android-35" \
    || true

bim-package-and-install "$install_dir" android-sdk "$version" "$build_type"
