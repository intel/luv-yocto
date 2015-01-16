SUMMARY = "Firmware testsuite"
DESCRIPTION = "The tool fwts comprises of tests that are designed to exercise BIOS, these need access to read BIOS data and ACPI tables"
HOMEPAGE = "https://wiki.ubuntu.com/Kernel/Reference/fwts"
LICENSE = "GPLv2+"
LIC_FILES_CHKSUM = "file://src/main.c;beginline=1;endline=16;md5=deb8af5388e838d133eaa036f4d1496f"

PV = "14.03.01+git${SRCPV}"

SRCREV = "8ec44dc1e55ecf334f4afa8eed8795ed5776c396"
SRC_URI = "git://kernel.ubuntu.com/hwe/fwts.git \
           file://luv-parser-fwts \
           file://0001-efi_runtime-Set-default-value-for-KVER.patch \
           file://0002-efi_runtime-Refactor-ioctl-code-into-helper-function.patch \
           file://0003-efi_runtime-Group-kernel-version-dependent-functions.patch \
           file://0004-efi_runtime-Do-not-pass-user-addresses-to-firmware.patch \
	   file://0005-efi_runtime-donot-dereference-user-address.patch \
           file://0006-cpu-microcode-remove-failures-when-kernel-does-not-h.patch \ 
           file://0007-securebootcert-report-info-instead-of-failure-for-mi.patch \
           file://0008-efi_runtime-Don-t-use-get_user-on-non-pointer.patch \
           file://0009-efi_runtime-limit-the-amount-of-data-copied-to-users.patch \
           file://0010-efi_runtime-get_nextvariable-copy-only-the-needed-na.patch \
          "

S = "${WORKDIR}/git"
DEPENDS = "autoconf automake libtool libpcre libjson flex bison \
	virtual/kernel "

inherit autotools luv-test module-base

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
