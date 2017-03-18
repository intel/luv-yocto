SUMMARY = "BIOSBits is a test suite that runs UEFI BIOS tests."

DESCRIPTION = " The Intel BIOS Implementation Test Suite (BITS) provides a bootable \
pre-OS environment for testing BIOSes and in particular their initialization \
of Intel processors, hardware, and technologies. BITS can verify your BIOS \
against many Intel recommendations. In addition, BITS includes Intel's \
official reference code as provided to BIOS, which you can use to override \
your BIOS's hardware initialization with a known-good configuration, and then boot an OS. \
BITS uses a particular version of Python that is maintained as part of the BITS source \
code. In regular BITS builds, this Python is available within the build directory. \
However, in order to build within bitbake, this Python needs to be available as part \
of the nativesdk."

# Home Page
HOMEPAGE = "http://biosbits.org/"

inherit native

#License
LICENSE = "GPLv3"
LIC_FILES_CHKSUM = "file://COPYING;md5=71a9ec458a3c65c2bfb461b227ef3049"

BITSVERSION="2079"
PV="${BITSVERSION}+git${SRCPV}"

S = "${WORKDIR}/git"

SRCREV ="19da7046a7303f1de8b53165eea1a6f486757c03"
SRC_URI = "gitsm://github.com/biosbits/bits.git;protocol=http  \
           file://BITS-python-_ctypes-do-not-use-the-WIN-64-EFI64-call.patch \
           file://BITS-python-configure-customize-Python-version.patch \
           file://Makefile-customize-the-Python-install-bin-directory.patch \
          "

S = "${WORKDIR}/git"

DEPENDS = "virtual/gettext autogen-native gettext-native sqlite3-native"

COMPATIBLE_HOST = '(x86_64.*|i.86.*)-(linux|freebsd.*)'

do_compile() {
	# adjust the target directories
	sed -i 's|BUILD_SYS|${BUILD_SYS}|' ${S}/Makefile
	sed -i 's|HOST_SYS|${HOST_SYS}|' ${S}/Makefile
	sed -i 's|TARGET_SYS|${TARGET_SYS}|' ${S}/Makefile
	sed -i 's|_PREFIX_|${prefix}|' ${S}/Makefile
	sed -i 's|EXEC_PREFIX|${exec_prefix}|' ${S}/Makefile
	sed -i 's|_BINDIR/PN|${bindir}/${PN}|' ${S}/Makefile
	sed -i 's|SBINDIR|${sbindir}/${PN}|' ${S}/Makefile
	sed -i 's|LIBEXECDIR|${libexecdir}|' ${S}/Makefile
	sed -i 's|DATADIR|${datadir}|' ${S}/Makefile
	sed -i 's|SYSCONFDIR|${sysconfdir}|' ${S}/Makefile
	sed -i 's|SHAREDSTATEDIR|${sharedstatedir}|' ${S}/Makefile
	sed -i 's|LOCALSTATEDIR|${localstatedir}|' ${S}/Makefile
	sed -i 's|LIBDIR|${libdir}|' ${S}/Makefile
	sed -i 's|_INCLUDEDIR|${includedir}|' ${S}/Makefile
	sed -i 's|OLDINCLUDEDIR|${oldincludedir}|' ${S}/Makefile
	sed -i 's|INFODIR|${infodir}|' ${S}/Makefile
	sed -i 's|MANDIR|${mandir}|' ${S}/Makefile

	oe_runmake build-python-host
}

do_install() {

	install -d ${D}${bindir}/${PN}
        oe_runmake -C ${B}/build/python-host/ 'DESTDIR=${D}' altinstall

        # Make sure we use /usr/bin/env python
        for PYTHSCRIPT in `grep -rIl ${bindir}/${PN}/python ${D}${bindir}/${PN}`; do
                sed -i -e '1s|^#!.*|#!/usr/bin/env python|' $PYTHSCRIPT
        done

        # Add a symlink to the native Python so that scripts can just invoke
        # "nativepython" and get the right one without needing absolute paths
        # (these often end up too long for the #! parser in the kernel as the
        # buffer is 128 bytes long).
        ln -s ${PN}/python2.7-BITS ${D}${bindir}/bitspython

        # We don't want modules in ~/.local being used in preference to those
        # installed in the native sysroot, so disable user site support.
        sed -i -e 's,^\(ENABLE_USER_SITE = \).*,\1False,' ${D}${libdir}/python2.7-BITS/site.py

}

