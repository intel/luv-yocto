SUMMARY = "Signing utility for UEFI secure boot"

LICENSE = "GPLv3"
LIC_FILES_CHKSUM = "file://LICENSE.GPLv3;md5=9eef91148a9b14ec7f9df333daebc746"

SRC_URI = "gitsm://kernel.ubuntu.com/jk/sbsigntool;protocol=git \
    file://fix-mixed-implicit-and-normal-rules.patch;apply=1 \
    file://disable-man-page-creation.patch \
    file://0001-configure-fix-cross-compilation.patch \
"

SRCREV="${AUTOREV}"

inherit autotools-brokensep pkgconfig

BBCLASSEXTEND = "native"

DEPENDS = "binutils openssl10 gnu-efi util-linux"

S = "${WORKDIR}/git"

do_configure() {
    OLD_CC="${CC}"

    if [ ! -e lib/ccan ]; then
        export CC="${BUILD_CC}"
        lib/ccan.git/tools/create-ccan-tree \
            --build-type=automake lib/ccan \
                talloc read_write_all build_assert array_size
    fi

    export CC="${OLD_CC}"
    export CFLAGS="-I${STAGING_INCDIR}/efi -I${STAGING_INCDIR}/efi/x86_64"
    ./autogen.sh --noconfigure
    oe_runconf
}
