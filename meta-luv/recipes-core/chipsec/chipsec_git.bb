SUMMARY = "CHIPSEC: Platform Security Assessment Framework"
DESCRIPTION = "CHIPSEC is a framework for analyzing security of PC \
platforms including hardware, system firmware including BIOS/UEFI \
and the configuration of platform components."

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8c16666ae6c159876a0ba63099614381"

SRC_URI = "git://github.com/chipsec/chipsec.git \
    file://0006-drivers-linux-Don-t-build-userland-app-automatically.patch \
    file://chipsec file://luv-parser-chipsec \
    file://fix-setup.py-for-Linux.patch \
    file://chipsec-setup-install-cores-library-under-helper-lin.patch \
    file://0001-chipsec-building-for-32-bit-systems.patch \
    file://0002-chipsec_km-utilize-inode_lock-unlock-wrappers-for-ne.patch"

SRCREV="20cc5a30675548a764dadfe0dc677a283816906c"
PV="1.2.2"

DEPENDS = "virtual/kernel python-core nasm-native"
RDEPENDS_${PN} = "python python-shell python-stringold python-xml \
    python-ctypes python-fcntl python-importlib"

COMPATIBLE_HOST='(i.86|x86_64).*'

inherit module-base
inherit python-dir
inherit distutils
inherit luv-test

S = "${WORKDIR}/git"

export INC = "-I${STAGING_INCDIR}/${PYTHON_DIR}"

def get_target_arch(d):
 import re
 target = d.getVar('TARGET_ARCH', True)
 if target == "x86_64":
    return 'x86_64'
 elif re.match('i.86', target):
    return 'i386'
 elif re.match('arm', target):
    return 'arm'
 else:
    raise bb.parse.SkipPackage("TARGET_ARCH %s not supported!" % target)

EXTRA_OEMAKE += "ARCH="${@get_target_arch(d)}""

fix_mod_path() {
    sed -i -e "s:^INSTALL_MOD_PATH_PREFIX = .*:INSTALL_MOD_PATH_PREFIX = \"${PYTHON_SITEPACKAGES_DIR}\":" ${S}/source/tool/chipsec_main.py
    sed -i -e "s:PYTHONPATH:${PYTHON_SITEPACKAGES_DIR}:" ${WORKDIR}/chipsec
}

do_patch_append() {
    bb.build.exec_func('fix_mod_path', d)
}

do_compile_prepend() {
    cd ${S}/source/tool
}

do_compile_append() {
    unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS

    oe_runmake KERNEL_SRC_DIR=${STAGING_KERNEL_DIR}   \
        KERNEL_VERSION=${KERNEL_VERSION}    \
        CC="${KERNEL_CC}" LD="${KERNEL_LD}" \
        AR="${KERNEL_AR}" -C ${STAGING_KERNEL_DIR} \
        scripts

    oe_runmake KERNEL_SRC_DIR=${STAGING_KERNEL_DIR}   \
        KERNEL_VERSION=${KERNEL_VERSION}    \
        CC="${KERNEL_CC}" LD="${KERNEL_LD}" \
        AR="${KERNEL_AR}" INC="${INC}" -C ${S}/source/drivers/linux

    oe_runmake -C ${S}/source/tool/chipsec/helper/linux
}

do_install_prepend() {
    cd ${S}/source/tool
}

do_install_append() {
    unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS

    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/chipsec ${D}${bindir}

    # Install the kernel driver
    oe_runmake DEPMOD=echo INSTALL_MOD_PATH="${D}" \
        KERNEL_SRC=${STAGING_KERNEL_DIR} \
        CC="${KERNEL_CC}" LD="${KERNEL_LD}" \
        -C ${STAGING_KERNEL_DIR} \
        M="${S}/source/drivers/linux" \
        modules_install
}

LUV_TEST_LOG_PARSER="luv-parser-chipsec"

FILES_${PN} += "/lib/modules/${KERNEL_VERSION}/extra/chipsec.ko"
FILES_${PN}-dbg +="${libdir}/${PYTHON_DIR}/site-packages/${PN}/helper/linux/.debug"
