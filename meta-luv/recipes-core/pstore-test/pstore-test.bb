DESCRIPTION = "pstore tests"
HOMEPAGE = "https://www.kernel.org/pub/linux/kernel"
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${STAGING_KERNEL_DIR}/COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"
KBRANCH="stable"

# Picking up matts branch
SRC_URI = "file://pstore-test-runner \
           file://luv-parser-pstore-test \
           file://pstore-test.json"

#we need some of the stuff below
DEPENDS_class-native += "qemu-native"
SRCREV="${AUTOREV}"
inherit autotools luv-test

do_fetch[noexec] = "1"
do_unpack[depends] += "virtual/kernel:do_shared_workdir"
do_patch[depends] += "virtual/kernel:do_shared_workdir"
do_package[depends] += "virtual/kernel:do_populate_sysroot"

do_unpack_append() {
    bb.build.exec_func('unpack_test_code', d)
}

unpack_test_code() {
    mkdir -p ${S}/src/pstore
    cp -pRv ${STAGING_KERNEL_DIR}/tools/testing/selftests/pstore/* ${S}/src/pstore
    cp -pRv ${STAGING_KERNEL_DIR}/tools/testing/selftests/lib.mk ${S}/src
}

# If the LUV_STORAGE variable in luv_test_manager.bb ever changes,
# the reboot_dir variable should be changed accordingly
reboot_dir = "/mnt/luv-storage"

EXTRA_OEMAKE = " \
    CC='${CC}' \
    -C ${S}/src/pstore"

patch() {
       sed -i 's,REBOOT_DIR,${reboot_dir},g' ${S}/src/pstore/common_tests
       sed -i 's,REBOOT_DIR,${reboot_dir},g' ${WORKDIR}/luv-parser-pstore-test
       sed -i 's,REBOOT_DIR,${reboot_dir},g' ${WORKDIR}/pstore-test-runner
}

do_patch_append() {
    bb.build.exec_func('patch', d)
}

do_configure_prepend() {
    # We need to ensure the --sysroot option in CC is preserved
    if [ -e "${S}/src/pstore/Makefile" ]; then
        sed -i 's,CC = $(CROSS_COMPILE)gcc,#CC,' ${S}/src/pstore/Makefile
    fi

    # Fix for rebuilding
    oe_runmake clean
}

# Installing is nothing but putting things in place
do_install() {
    # Creating a directory
    install -d ${D}${datadir}/pstore-test

    # Copying some of the files, these are part of the linux code
    install -m 0755 ${S}/src/pstore/common_tests ${D}${datadir}/pstore-test
    install -m 0755 ${S}/src/pstore/pstore_crash_test ${D}${datadir}/pstore-test
    install -m 0755 ${S}/src/pstore/pstore_post_reboot_tests ${D}${datadir}/pstore-test
    install -m 0755 ${S}/src/pstore/pstore_tests ${D}${datadir}/pstore-test

    # This is the script which will run all the tests
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/pstore-test-runner ${D}${bindir}
}

FILES_${PN} += "/usr/share/pstore-test/common_tests \
               /usr/share/pstore-test/pstore_crash_test \
               /usr/share/pstore-test/pstore_post_reboot \
               /usr/share/pstore-test/pstore_tests \
               ${bindir}/pstore-test-runner \
               "

LUV_TEST_LOG_PARSER="luv-parser-pstore-test"
LUV_TEST_JSON="pstore-test.json"
LUV_TEST="pstore-test-runner"
LUV_TEST_ARGS=""
