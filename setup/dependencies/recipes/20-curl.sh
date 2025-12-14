#!/bin/bash

set -euo pipefail

[[ "${bim_target_platform:-}" == "linux" ]] || exit 0

curl_version=8.17.0
package_revision=1
version="$curl_version"-"$package_revision"
build_type=release

! bim-install-package curl "$version" "$build_type" 2>/dev/null \
    || exit 0

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")"; pwd)"

# shellcheck source=SCRIPTDIR/set-package-vars.sh
. "$script_dir"/set-package-vars.sh curl "$build_type"

rm --force --recursive "$build_dir" "$install_dir"
mkdir --parents "$source_dir" "$build_dir" "$install_dir"

archive_base_name=curl-"${curl_version}"
archive_name="${archive_base_name}".tar.bz2
archive_url="https://github.com/curl/curl/releases/download/curl-${curl_version//./_}/${archive_name}"

cd "$source_dir"

download \
    --url="$archive_url" \
    --target-file="$archive_name" \
    --mime-type=application/x-bzip2

tar xf "$archive_name"

cd "$build_dir"

bim-cmake-build \
    --build-dir "$build_dir" \
    --build-type "${build_type^}" \
    --install-dir "$install_dir" \
    --source-dir "$source_dir"/"$archive_base_name" \
    --cmake -DBUILD_CURL_EXE=OFF \
    --cmake -DBUILD_EXAMPLES=OFF \
    --cmake -DBUILD_LIBCURL_DOCS=OFF \
    --cmake -DBUILD_MISC_DOCS=OFF \
    --cmake -DBUILD_SHARED_LIBS=OFF \
    --cmake -DBUILD_STATIC_LIBS=OFF \
    --cmake -DBUILD_TESTING=OFF \
    --cmake -DCURL_BROTLI=OFF \
    --cmake -DCURL_CA_PATH_SET=FALSE \
    --cmake -DCURL_DISABLE_ALTSVC=TRUE \
    --cmake -DCURL_DISABLE_AWS=TRUE \
    --cmake -DCURL_DISABLE_BASIC_AUTH=TRUE \
    --cmake -DCURL_DISABLE_BEARER_AUTH=TRUE \
    --cmake -DCURL_DISABLE_BINDLOCAL=TRUE \
    --cmake -DCURL_DISABLE_COOKIES=TRUE \
    --cmake -DCURL_DISABLE_DICT=TRUE \
    --cmake -DCURL_DISABLE_DIGEST_AUTH=TRUE \
    --cmake -DCURL_DISABLE_DOH=TRUE \
    --cmake -DCURL_DISABLE_FILE=TRUE \
    --cmake -DCURL_DISABLE_FORM_API=TRUE \
    --cmake -DCURL_DISABLE_FTP=TRUE \
    --cmake -DCURL_DISABLE_GETOPTIONS=TRUE \
    --cmake -DCURL_DISABLE_GOPHER=TRUE \
    --cmake -DCURL_DISABLE_HEADERS_API=TRUE \
    --cmake -DCURL_DISABLE_HSTS=TRUE \
    --cmake -DCURL_DISABLE_HTTP=FALSE \
    --cmake -DCURL_DISABLE_HTTP_AUTH=TRUE \
    --cmake -DCURL_DISABLE_IMAP=TRUE \
    --cmake -DCURL_DISABLE_IPFS=TRUE \
    --cmake -DCURL_DISABLE_KERBEROS_AUTH=TRUE \
    --cmake -DCURL_DISABLE_LDAP=TRUE \
    --cmake -DCURL_DISABLE_LDAPS=TRUE \
    --cmake -DCURL_DISABLE_LIBCURL_OPTION=TRUE \
    --cmake -DCURL_DISABLE_MIME=TRUE \
    --cmake -DCURL_DISABLE_MQTT=TRUE \
    --cmake -DCURL_DISABLE_NEGOTIATE_AUTH=TRUE \
    --cmake -DCURL_DISABLE_NETRC=TRUE \
    --cmake -DCURL_DISABLE_NTLM=TRUE \
    --cmake -DCURL_DISABLE_PARSEDATE=TRUE \
    --cmake -DCURL_DISABLE_POP3=TRUE \
    --cmake -DCURL_DISABLE_PROGRESS_METER=TRUE \
    --cmake -DCURL_DISABLE_PROXY=TRUE \
    --cmake -DCURL_DISABLE_RTSP=TRUE \
    --cmake -DCURL_DISABLE_SHA512_256=TRUE \
    --cmake -DCURL_DISABLE_SHUFFLE_DNS=TRUE \
    --cmake -DCURL_DISABLE_SMB=TRUE \
    --cmake -DCURL_DISABLE_SMTP=TRUE \
    --cmake -DCURL_DISABLE_SOCKETPAIR=TRUE \
    --cmake -DCURL_DISABLE_SRP=TRUE \
    --cmake -DCURL_DISABLE_TELNET=TRUE \
    --cmake -DCURL_DISABLE_TFTP=TRUE \
    --cmake -DCURL_DISABLE_VERBOSE_STRINGS=TRUE \
    --cmake -DCURL_DISABLE_WEBSOCKETS=TRUE \
    --cmake -DCURL_ENABLE_EXPORT_TARGET=TRUE \
    --cmake -DCURL_HIDDEN_SYMBOLS=ON \
    --cmake -DCURL_USE_GSASL=FALSE \
    --cmake -DCURL_USE_GSSAPI=FALSE \
    --cmake -DCURL_USE_LIBPSL=FALSE \
    --cmake -DCURL_USE_LIBSSH2=FALSE \
    --cmake -DCURL_USE_LIBSSH=FALSE \
    --cmake -DCURL_ZLIB=OFF \
    --cmake -DCURL_ZSTD=OFF \
    --cmake -DENABLE_CURL_MANUAL=OFF \
    --cmake -DUSE_ECH=OFF \
    --cmake -DUSE_HTTPSRR=OFF \
    --cmake -DUSE_LIBIDN2=OFF \
    --cmake -DUSE_LIBRTMP=OFF \
    --cmake -DUSE_NGHTTP2=OFF \
    --cmake -DUSE_NGTCP2=OFF \
    --cmake -DUSE_OPENSSL_QUIC=OFF \
    --cmake -DUSE_QUICHE=OFF \
    --cmake -DUSE_SSLS_EXPORT=OFF

rm "$install_dir"/bin/wcurl

bim-package-and-install "$install_dir" curl "$version" "$build_type"
