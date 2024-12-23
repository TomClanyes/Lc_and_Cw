#!/bin/sh
md5sum=/usr/bin/md5sum
sha1sum=/usr/bin/sha1sum
sha256sum=/usr/bin/sha256sum

set -e

do_hash() {
    HASH_NAME=$1
    HASH_CMD=$2
    echo "${HASH_NAME}:"
    for f in $(find -type f -name "*.deb"); do
        f=$(echo $f | cut -c3-)
        echo " $(${HASH_CMD} ${f}  | cut -d" " -f1) $(wc -c $f)"
    done
}

cat << EOF
Origin: MySysLog Repository
Label: MySysLog
Suite: stable
Codename: stable
Version: 1.0
Architectures: amd64 arm64
Components: main
Description: Repo with MySysLog packages
Date: $(date -Ru)
EOF
do_hash "MD5Sum" "$md5sum"
do_hash "SHA1" "$sha1sum"
do_hash "SHA256" "$sha256sum"
