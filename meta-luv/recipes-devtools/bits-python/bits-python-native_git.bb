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

BITSVERSION="2073"
PV="${BITSVERSION}+git${SRCPV}"

S = "${WORKDIR}/git"

SRCREV ="5931fde3bc7da376a33411e07423722977502259"
SRC_URI = "gitsm://github.com/biosbits/bits.git;protocol=http  \
           file://BITS-python-_ctypes-do-not-use-the-WIN-64-EFI64-call.patch \
           file://BITS-python-configure-customize-Python-version.patch \
           file://Makefile-customize-the-Python-install-bin-directory.patch \
          "

S = "${WORKDIR}/git"

#EXTRA_OEMAKE = '\
#  LIBC="" \
#  STAGING_LIBDIR=${STAGING_LIBDIR_NATIVE} \
#  STAGING_INCDIR=${STAGING_INCDIR_NATIVE} \
#'


DEPENDS = "virtual/gettext autogen-native gettext-native sqlite3-native"

COMPATIBLE_HOST = '(x86_64.*|i.86.*)-(linux|freebsd.*)'

do_compile() {
	# adjust the install bin directory
	sed -i 's|BINDIR/PN|${bindir}/${PN}|' Makefile
	sed -i 's|CUSTOM_PREFIX_DIR|${STAGING_DIR_NATIVE}/usr|' Makefile
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

