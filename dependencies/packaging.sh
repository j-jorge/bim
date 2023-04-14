# -*- sh -*-

install_package()
{
    local name="$1"
    local version="$2"

    paco-install --disable-remote \
                 --name "$name" \
                 --version "$version" \
                 --prefix "$bomb_app_prefix" \
                 --flavor "$bomb_build_type" \
                 --platform linux
}

package_and_install()
{
    local install_dir="$1"
    local name="$2"
    local version="$3"

    paco-publish --disable-remote \
                 --root "$install_dir" \
                 --name "$name" \
                 --version "$version" \
                 --flavor "$bomb_build_type" \
                 --platform linux

    install_package "$name" "$version"
}
