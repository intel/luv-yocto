SUMMARY = "Firmware testsuite"
DESCRIPTION = "The tool fwts comprises of tests that are designed to exercise BIOS, these need access to read BIOS data and ACPI tables"
HOMEPAGE = "https://wiki.ubuntu.com/Kernel/Reference/fwts"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://src/main.c;beginline=1;endline=16;md5=deb8af5388e838d133eaa036f4d1496f"

PV = "14.03.01+git${SRCPV}"

SRCREV = "8ec44dc1e55ecf334f4afa8eed8795ed5776c396"
SRC_URI = "git://kernel.ubuntu.com/hwe/fwts.git \
           file://luv-parser-fwts \
          "

S = "${WORKDIR}/git"
DEPENDS = "autoconf automake libtool libpcre libjson flex bison "

inherit autotools luv-test

LUV_TEST_LOG_PARSER="luv-parser-fwts"
LUV_TEST_ARGS="-r stdout -q --uefi --log-filter='SUM,INF' \
	--log-format='%owner;%field '"

FILES_${PN} += "${libdir}/fwts/lib*${SOLIBS}"
FILES_${PN}-dev += "${libdir}/fwts/lib*${SOLIBSDEV} ${libdir}/fwts/lib*.la"
