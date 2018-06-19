DESCRIPTION = "EFI varfs tests"
HOMEPAGE = "https://www.kernel.org/pub/linux/kernel"
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${STAGING_KERNEL_DIR}/COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"
KBRANCH="stable"

# Picking up matts branch
SRC_URI = "file://luv-parser-efivarfs \
           file://efivarfs \
           file://efivarfs.json"

#we need some of the stuff below
DEPENDS_class-native += "qemu-native"
SRCREV="${AUTOREV}"
inherit autotools luv-test

RDEPENDS_${PN} += "e2fsprogs bash"

do_fetch[noexec] = "1"
do_unpack[depends] += "virtual/kernel:do_shared_workdir"
do_patch[depends] += "virtual/kernel:do_shared_workdir"
do_package[depends] += "virtual/kernel:do_populate_sysroot"

do_unpack_append() {
    bb.build.exec_func('unpack_test_code', d)
}
unpack_test_code() {
    mkdir -p ${S}/src/efivarfs
    cp -pRv ${STAGING_KERNEL_DIR}/tools/testing/selftests/efivarfs/* ${S}/src/efivarfs
    cp -pRv ${STAGING_KERNEL_DIR}/tools/testing/selftests/lib.mk ${S}/src
}

EXTRA_OEMAKE = " \
    CC='${CC}' \
    -C ${S}/src/efivarfs"

# This is to just to satisfy the compilation error
#I am not sure why I am getting this
FILES_${PN}-dbg += "/usr/share/efivarfs-test/.debug"

do_configure_prepend() {
    # We need to ensure the --sysroot option in CC is preserved
    if [ -e "${S}/src/efivarfs/Makefile" ]; then
        sed -i 's,CC = $(CROSS_COMPILE)gcc,#CC,' ${S}/src/efivarfs/Makefile
    fi

    # Fix for rebuilding
    oe_runmake clean
}

#This is the compilation area
#we need to compile the self tests
do_compile() {
    unset CFLAGS
    oe_runmake
}


#Installing is nothing but putting things in place
do_install() {
    # Creating a directory
    install -d ${D}${datadir}/efivarfs-test

    #Copying some of the files, these are part of the linux code
    install -m 0755 ${S}/src/efivarfs/create-read ${D}${datadir}/efivarfs-test
    install -m 0755 ${S}/src/efivarfs/open-unlink ${D}${datadir}/efivarfs-test
    install -m 0755 ${S}/src/efivarfs/efivarfs.sh ${D}${datadir}/efivarfs-test

    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/efivarfs ${D}${bindir}
}

LUV_TEST_LOG_PARSER="luv-parser-efivarfs"
LUV_TEST_JSON="efivarfs.json"
LUV_TEST="efivarfs"
