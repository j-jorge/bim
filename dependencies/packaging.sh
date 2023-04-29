# -*- sh -*-

install_package()
{
    local name="$1"
    local version="$2"
    local flavor="$3"

    paco-install --disable-remote \
                 --name "$name" \
                 --version "$version" \
                 --prefix "$bomb_app_prefix" \
                 --flavor "$flavor" \
                 --platform linux
}

package_and_install()
{
    local install_dir="$1"
    local name="$2"
    local version="$3"
    local flavor="$4"

    paco-publish --disable-remote \
                 --root "$install_dir" \
                 --name "$name" \
                 --version "$version" \
                 --flavor "$flavor" \
                 --platform linux

    install_package "$name" "$version" "$flavor"
}
