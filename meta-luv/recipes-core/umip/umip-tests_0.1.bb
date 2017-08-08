SUMMARY = "Tests for Intel User-Mode Instruction Prevention"
DESCRIPTION = "Use the instructions covered by UMIP and make sure they are handled correctly"
HOMEPAGE = "https://www.github.com/ricardon"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=751419260aa954499f7abaabaa882bbe"


SRC_URI="file://umip_test_defs.h \
         file://umip_utils.c \
         file://umip_test_basic.c \
         file://umip_test_opnds.c \
         file://umip_exceptions.c \
         file://COPYING \
         file://Makefile \
         file://umip_ldt_32.c \
         file://umip_test_gen_32.py \
         file://umip_ldt_16.c \
         file://umip_test_gen_16.py \
         file://umip_ldt_64.c \
         file://umip_test_gen_64.py \
         file://UMIP_README \
        "


do_configure[noexec] = "1"

EXTRA_OEMAKE += " ${TARGET_ARCH}"

S = "${WORKDIR}"

do_compile() {
	oe_runmake
}

do_install() {
        install -d ${D}${bindir}
        install -m 744 ${WORKDIR}/UMIP_README ${D}${bindir}
        if [ "${TARGET_ARCH}" = "x86_64" ]; then
                install -m 755 ${WORKDIR}/umip_test_basic_64 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_exceptions_64 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_test_opnds_64 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_ldt_64 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_test_basic_64_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_exceptions_64_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_test_opnds_64_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_ldt_64_emul_all ${D}${bindir}
        fi
        if [ "${TARGET_ARCH}" = "i586" ]; then
                install -m 755 ${WORKDIR}/umip_test_basic_32 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_exceptions_32 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_test_opnds_32 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_ldt_32 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_ldt_16 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_test_basic_32_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_exceptions_32_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_test_opnds_32_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_ldt_32_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_ldt_16_emul_all ${D}${bindir}
        fi
        if [ "${TARGET_ARCH}" = "i686" ]; then
                install -m 755 ${WORKDIR}/umip_test_basic_32 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_exceptions_32 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_test_opnds_32 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_ldt_32 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_ldt_16 ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_test_basic_32_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_exceptions_32_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_test_opnds_32_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_ldt_32_emul_all ${D}${bindir}
                install -m 755 ${WORKDIR}/umip_ldt_16_emul_all ${D}${bindir}
        fi

}


