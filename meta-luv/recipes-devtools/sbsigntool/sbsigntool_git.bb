SUMMARY = "Signing utility for UEFI secure boot"

LICENSE = "GPLv3"
LIC_FILES_CHKSUM = "file://LICENSE.GPLv3;md5=9eef91148a9b14ec7f9df333daebc746"

SRC_URI = "git://kernel.ubuntu.com/jk/sbsigntool \
    file://fix-mixed-implicit-and-normal-rules.patch;apply=0 \
    file://disable-man-page-creation.patch"

SRCREV="${AUTOREV}"

inherit autotools-brokensep pkgconfig

BBCLASSEXTEND = "native"

DEPENDS = "binutils openssl gnu-efi util-linux"

S = "${WORKDIR}/git"

do_configure_class-native() {
    if [ ! -e lib/ccan.git/Makefile ]; then
        git submodule init
        git submodule update

        patch -p1 -i ../fix-mixed-implicit-and-normal-rules.patch
    fi

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
