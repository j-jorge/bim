#!/bin/bash

: "${bim_packages_root:-}"
: "${bim_package_install_platform:-}"

source_dir="$bim_packages_root"/"$1"/source
build_dir="$bim_packages_root"/"$1"/build-"$bim_package_install_platform"-"$2"
install_dir="$bim_packages_root"/"$1"/install-"$bim_package_install_platform"-"$2"

: "${source_dir:-}"
: "${build_dir:-}"
: "${install_dir:-}"

