SUMMARY = "Firmware testsuite"
DESCRIPTION = "The tool fwts comprises of tests that are designed to exercise BIOS, these need access to read BIOS data and ACPI tables"
HOMEPAGE = "https://wiki.ubuntu.com/Kernel/Reference/fwts"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://src/main.c;beginline=1;endline=16;md5=9bd929284041f05a42b0b6a7b1864a31"

PV = "15.03.01+git${SRCPV}"

SRCREV = "aba49d907484b153071320d396407bd35c0e4123"
SRC_URI = "git://kernel.ubuntu.com/hwe/fwts.git \
           file://luv-parser-fwts \
           file://uefi-efi_runtime-eliminate-confusing-error-message-of-get_variable.patch \
          "

S = "${WORKDIR}/git"
DEPENDS = "autoconf automake libtool libpcre json-c flex bison \
	virtual/kernel glib-2.0"

inherit autotools-brokensep luv-test module-base

CFLAGS += "-I${STAGING_INCDIR}/json"

do_unpack[depends] += "virtual/kernel:do_populate_sysroot"

do_compile_append() {
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS
	oe_runmake KERNEL_PATH=${STAGING_KERNEL_DIR}   \
		KERNEL_SRC=${STAGING_KERNEL_DIR}    \
		KERNEL_VERSION=${KERNEL_VERSION}    \
		CC="${KERNEL_CC}" LD="${KERNEL_LD}" \
		AR="${KERNEL_AR}" -C ${STAGING_KERNEL_DIR} \
		scripts

	oe_runmake KERNEL_PATH=${STAGING_KERNEL_DIR}   \
		KERNEL_SRC=${STAGING_KERNEL_DIR}    \
		KERNEL_VERSION=${KERNEL_VERSION}    \
		CC="${KERNEL_CC}" LD="${KERNEL_LD}" \
		AR="${KERNEL_AR}" -C ${STAGING_KERNEL_DIR} \
		M="${S}/efi_runtime" \
		modules
}

do_install_append() {
	unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS
	oe_runmake DEPMOD=echo INSTALL_MOD_PATH="${D}" \
		KERNEL_SRC=${STAGING_KERNEL_DIR} \
		CC="${KERNEL_CC}" LD="${KERNEL_LD}" \
		-C ${STAGING_KERNEL_DIR} \
		M="${S}/efi_runtime" \
		modules_install
}

LUV_TEST_LOG_PARSER="luv-parser-fwts"
LUV_TEST_ARGS="-r stdout -q --uefi --log-filter='SUM,INF' \
	--log-format='%owner;%field ' --batch"

FILES_${PN} += "${libdir}/fwts/lib*${SOLIBS}"
FILES_${PN} += "/lib/modules/${KERNEL_VERSION}/extra/efi_runtime.ko"
FILES_${PN}-dev += "${libdir}/fwts/lib*${SOLIBSDEV} ${libdir}/fwts/lib*.la"
