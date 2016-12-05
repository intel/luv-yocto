SUMMARY = "Tests for Intel User-Mode Instruction Prevention"
DESCRIPTION = "Use the instructions covered by UMIP and make sure they are handled correctly"
HOMEPAGE = "https://www.github.com/ricardon"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=751419260aa954499f7abaabaa882bbe"


SRC_URI="file://umip_test.c \
         file://umip_test2.c \
         file://normal_pf.c \
         file://COPYING \
         file://Makefile"

#do_configure[noexec] = "1"

S = "${WORKDIR}"

do_compile() {
	oe_runmake
}

do_install() {
        install -d ${D}${bindir}
        install -m 755 ${WORKDIR}/umip_test ${D}${bindir}
        install -m 755 ${WORKDIR}/umip_test2_32 ${D}${bindir}
        install -m 755 ${WORKDIR}/umip_test2_64 ${D}${bindir}
        install -m 755 ${WORKDIR}/umip_pf ${D}${bindir}
}


