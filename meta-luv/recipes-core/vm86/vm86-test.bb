DESCRIPTION = "virtual-8086 mode kernel selftests"
HOMEPAGE = "https://www.kernel.org/pub/linux/kernel"
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

S = "${STAGING_KERNEL_DIR}"

SRCREV="${AUTOREV}"
S = "${STAGING_KERNEL_DIR}"

do_fetch[noexec] = "1"
do_unpack[depends] += "virtual/kernel:do_unpack"
do_patch[depends] += "virtual/kernel:do_shared_workdir"
do_package[depends] += "virtual/kernel:do_populate_sysroot"

EXTRA_OEMAKE = " \
    CC='${CC}' \
    -C ${S}/tools/testing/selftests/x86"

#This is the compilation area
#we need to compile the self tests
do_compile() {
    unset CFLAGS
    oe_runmake
}


#Installing is nothing but putting things in place
do_install() {
    # Creating a directory

    install -d ${D}${bindir}
    install -m 0755 ${STAGING_KERNEL_DIR}/tools/testing/selftests/x86/entry_from_vm86_32 ${D}${bindir}
    install -m 0755 ${STAGING_KERNEL_DIR}/tools/testing/selftests/x86/protection_keys_32 ${D}${bindir}
}

#LUV_TEST_LOG_PARSER="luv-parser-efivarfs"
#LUV_TEST="efivarfs"
