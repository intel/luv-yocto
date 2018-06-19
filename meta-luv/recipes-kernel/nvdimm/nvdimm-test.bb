DESCRIPTION = "nvdimm tests"
HOMEPAGE = "https://www.kernel.org/pub/linux/kernel"
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${STAGING_KERNEL_DIR}/COPYING;md5=bbea815ee2795b2f4230826c0c6b8814"
KBRANCH="stable"

inherit module

SRC_URI = "file://Makefile"

S = "${WORKDIR}"

EXTRA_OEMAKE += "KERNEL=${STAGING_KERNEL_DIR}"
KERNEL_MODULE_PACKAGE_PREFIX = "nvdimm-test"

do_install_prepend() {
        cp ${STAGING_KERNEL_DIR}/tools/testing/nvdimm/Module.symvers ${B}
}

do_populate_lic[depends] += "virtual/kernel:do_populate_lic"

RPROVIDES_${PN} = "kernel-module-libnvdimm"
