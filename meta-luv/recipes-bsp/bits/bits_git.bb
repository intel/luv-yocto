SUMMARY = "BIOSBits is a test suite that runs UEFI BIOS tests."

DESCRIPTION = " The Intel BIOS Implementation Test Suite (BITS) provides a bootable \
pre-OS environment for testing BIOSes and in particular their initialization \
of Intel processors, hardware, and technologies. BITS can verify your BIOS \
against many Intel recommendations. In addition, BITS includes Intel's \
official reference code as provided to BIOS, which you can use to override \
your BIOS's hardware initialization with a known-good configuration, and then boot an OS."

# Home Page
HOMEPAGE = "http://biosbits.org/"

#License
LICENSE = "GPLv3"
LIC_FILES_CHKSUM = "file://COPYING;md5=71a9ec458a3c65c2bfb461b227ef3049"

BBCLASSEXTEND = "native"

# CFLAGS has a -Werror which ensures all warnings are treated as errors.
# Since BITS has a dependency on grub and python and no one is actively
# maintaining it, resolving all these warnings consumes a lot of time.
# Thus, we have decided to not treat the warnings as errors.
CFLAGS_append = " -Wno-error "

BITSVERSION="2079"
PV="${BITSVERSION}+git${SRCPV}"

inherit deploy
inherit luv-test

# TODO: add patch for a CPIO log file
SRCREV ="19da7046a7303f1de8b53165eea1a6f486757c03"
SRC_URI = "gitsm://github.com/biosbits/bits.git;protocol=http  \
           file://GRUB-core-lib-crypto-complete-handling-fgets-return-.patch \
           file://GRUB-util-of-complete-handling-of-read-return-values.patch \
           file://GRUB-core-net-bootp-conserve-variable-qualifier.patch; \
           file://BITS-rc-python-Use-fwrap-to-build-python.patch \
           file://BITS-GRUB-adjust-configure-params-for-bitbake.patch \
           file://BITS-Makefile-split-grub-build-from-image-generation.patch \
           file://BITS-Makefile-do-not-depend-bytecompile-on-build-pyt.patch \
           file://BITS-Makefile-allow-incremental-builds.patch \
           file://BITS-bits-cfg.txt-Set-batch-mode.patch \
           file://BITS-init.cfg-Exit-BITS-upon-completion.patch \
           file://BITS-use-multiple-parallel-jobs-when-building.patch \
           file://BITS-python-redirect-add-support-for-cpio-archives.patch \
           file://BITS-specify-the-location-of-modules-for-mkimage.patch \
           file://BITS-save-log-to-a-cpio-archive.patch \
           file://GRUB-core-handle-loader-options.patch \
           file://BITS-GRUB-prepare-to-use-a-memdisk.patch \
           file://GRUB-efidisk-Respect-block_io_protocol-buffer-alignment.patch \
           file://GRUB-core-libgcrypt-fix-signed-vs-unsigned-comparison.patch \
           file://GRUB-core-libgcrypt-respect-aliasing-rules.patch \
           file://PYTHON-Avoid-shifts-of-negative-values-inflateMark.patch \
           file://PYTHON-ast-fix-indentation.patch \
           file://BITS-pyton-compat-fix-error-handling-lseek.patch \
           file://FDLIBM-fix-indentation-warnings.patch \
           file://BITS-bitfields-add-function-to-find-the-first-bit-set-in-.patch \
           file://BITS-smrr-Fix-rules-of-validity-in-SMRR-MSRs.patch \
           file://BITS-init.py-Enable-logging-debugging-in-bits-via-ne.patch \
           file://luv-test-bits \
           file://luv-parser-bits \
           file://bits.json \
           file://0001-only-output-to-log.patch;apply=no \
           file://GRUB-util-include-sysmacros.h-header.patch \
           file://GRUB-fix-switch-fallthrough-cases-with-GCC7.patch \
           file://GRUB-btrfs-avoid-used-uninitialized-error-with-GCC7.patch \
           file://PYTHON-Fix-gcc7-warnings-when-calling-PyMem_MALLOC-macros.patch \
           file://libffi-fix-switch-fallthrough-cases-with-GCC7.patch \
           file://PYTHON-Fix-switch-fallthrough-cases-with-GCC7.patch \
           file://BITS-toplevel.cfg-Disable-Bits-when-user-chose-not-to-run.patch \
           file://BITS-init.py-Add-a-new-unittest-MCE-tests-to-BITS.patch \
           file://BITS-toplevel.cfg-Disable-Bits-when-LUV_BITS_CAN_RUN-is-not-true.patch \
           file://BITS-Make-LUV_BITS_CAN_RUN-false-after-BITS-starts.patch \
          "

S = "${WORKDIR}/git"

DEPENDS = "virtual/gettext autogen-native gettext-native sqlite3-native zip-native \
           xorriso-native bits-native flex bison autogen-native"
RDEPENDS_${PN}_class-target = "coreutils findutils cpio"

COMPATIBLE_HOST = '(x86_64.*|i.86.*)-(linux|freebsd.*)'

# Determine the target arch for the bios modules before the native class
# clobbers TARGET_ARCH.

def get_bits_arch(d):
    import re
    target = d.getVar('TARGET_ARCH', True)
    if target == "x86_64":
        return 'x86_64'
    elif re.match('i.86', target):
        return 'i386'
    else:
        raise bb.parse.SkipPackage("TARGET_ARCH %s not supported!" % target)

BITS_ARCH = "${@get_bits_arch(d)}"


LUV_NETBOOT_MODULES = "${@bb.utils.contains('DISTRO_FEATURES', 'luv-netboot', "memdisk", "", d)}"
LUV_NETBOOT_ROOT = "${@bb.utils.contains('DISTRO_FEATURES', 'luv-netboot', "(memdisk)", "", d)}"

LUV_TEST_LOG_PARSER = "luv-parser-bits"
LUV_TEST_JSON = "bits.json"

do_deploy() {

       install -d ${DEPLOYDIR}/bits

       cp -r ${B}/build/bits-${BITSVERSION}/boot/ ${DEPLOYDIR}/bits/
       cp -r ${B}/build/bits-${BITSVERSION}/efi ${DEPLOYDIR}/bits/
}

do_deploy_class-native() {
        :
}

addtask deploy before do_build after do_install

patch() {
	sed -i s/BUILD_SYS/${BUILD_SYS}/ ${S}/Makefile
	sed -i s/TARGET_SYS/${TARGET_SYS}/ ${S}/Makefile
	sed -i s/HOST_SYS/${HOST_SYS}/ ${S}/Makefile
	sed -i s'/BITS_PARALLEL_JOBS/${PARALLEL_MAKE}/g' ${S}/Makefile
	sed -i s'/LUV_NETBOOT_MODULES/${LUV_NETBOOT_MODULES}/' ${S}/Makefile
	sed -i s'/LUV_NETBOOT_ROOT/${LUV_NETBOOT_ROOT}/' ${S}/Makefile

}

do_patch_append() {
    bb.build.exec_func('patch', d)
}

do_compile() {
	oe_runmake bytecompile-pylib
	oe_runmake bytecompile-bits-python
	oe_runmake build-grub-${BITS_ARCH}-efi
}


BITS_INSTALL_TARGETS ="install-doc install-grub-cfg install-log install-bitsversion \
                       install-bitsconfigdefaults install-toplevel-cfg install-bits-cfg \
                       install-readme install-news install-install install-copying"

do_install_class-target() {
	oe_runmake mkimage-grub-${BITS_ARCH}-efi
	oe_runmake ${BITS_INSTALL_TARGETS}

	# TODO: Do we need to distribute source code?
	# This target creates a tar ball of the source code. This directory can change
	# while this happens as other targets are built in parallel (and create their
	# own targets). Thus,we need to run this target separately.
	oe_runmake install-src-bits

	install -d ${D}${bindir}
	install -m 0755 ${WORKDIR}/luv-test-bits ${D}/${bindir}/bits
}

do_install_class-native() {
        install -d ${D}${bindir}
        install -m 755 ${S}/build/grub-inst-${BUILD_ARCH}-efi/bin/grub-mkimage ${D}${bindir}/bits-grub-mkimage
}

