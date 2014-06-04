SUMMARY = "CHIPSEC: Platform Security Assessment Framework"
DESCRIPTION = "CHIPSEC is a framework for analyzing security of PC \
platforms including hardware, system firmware including BIOS/UEFI \
and the configuration of platform components."

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE;md5=8c16666ae6c159876a0ba63099614381"

SRC_URI = "git://github.com/chipsec/chipsec.git \
    file://0001-helper-linux-Use-CC-instead-of-cc.patch \
    file://0002-helper-linux-Allow-INC-to-be-overidden.patch \
    file://0003-chipsec_main.py-Remove-absolute-reference-to-python-.patch \
    file://0005-tool-setup.py-Delete-Windows-drivers-from-data_files.patch \
    file://chipsec file://luv-parser-chipsec"

SRCREV="v1.1.0"

DEPENDS = "virtual/kernel python-core"
RDEPENDS_${PN} = "python python-shell python-stringold python-xml \
    python-ctypes python-fcntl python-importlib"

inherit module-base
inherit python-dir
inherit distutils
inherit luv-test

S = "${WORKDIR}/git"

export INC = "-I${STAGING_INCDIR}/${PYTHON_DIR}"

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
