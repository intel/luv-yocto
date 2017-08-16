SUMMARY = "libnvdimm utility library"

DESCRIPTION = "Utility library for managing the libnvdimm \
(non-volatile memory device) sub-system in the Linux kernel. \
The LIBNVDIMM subsystem provides support for three types of \
NVDIMMs, namely,PMEM, BLK, and NVDIMM devices that can \
simultaneously support both PMEM and BLK mode access."

HOMEPAGE = "https://git.kernel.org/cgit/linux/kernel/git/nvdimm/nvdimm.git/tree/Documentation/nvdimm/nvdimm.txt?h=libnvdimm-for-next"

LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://COPYING;md5=e66651809cac5da60c8b80e9e4e79e08"

PV = "v57+git${SRCPV}"

RDEPENDS_${PN} = "nvdimm-test"
COMPATIBLE_HOST='(x86_64).*'

inherit autotools-brokensep luv-test
inherit pkgconfig
inherit module-base

SRC_URI += "git://github.com/pmem/ndctl.git"
SRC_URI += "file://luv-parser-ndctl"
SRC_URI += "file://ndctl_runner"
SRCREV = "d685bff1ed574cc714ae3e47f9d913f1c074d2b2"

S = "${WORKDIR}/git"
DEPENDS = "virtual/kernel kmod udev"
EXTRA_OECONF += "--enable-test --enable-destructive --disable-docs"

do_configure_prepend() {
    ${S}/autogen.sh
}

do_install_append() {
        install -d ${D}${bindir}
        install -m 755 ${WORKDIR}/ndctl_runner ${D}${bindir}
}

FILES_${PN} += "/usr/share/bash-completion/completions/ndctl"
LUV_TEST_LOG_PARSER="luv-parser-ndctl"
LUV_TEST="ndctl_runner"
LUV_TEST_ARGS=""
