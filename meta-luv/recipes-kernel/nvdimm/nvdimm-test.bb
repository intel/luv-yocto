DESCRIPTION = "nvdimm tests"
HOMEPAGE = "https://www.kernel.org/pub/linux/kernel"
SECTION = "base"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${STAGING_KERNEL_DIR}/COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"
KBRANCH="stable"

inherit module

SRC_URI = "file://Makefile"

S = "${WORKDIR}"

EXTRA_OEMAKE += "KERNEL=${STAGING_KERNEL_DIR}"
