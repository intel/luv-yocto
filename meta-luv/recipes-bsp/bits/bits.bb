
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
LIC_FILES_CHKSUM = "file://boot/COPYING;md5=71a9ec458a3c65c2bfb461b227ef3049"

PV="1084"

inherit autotools
inherit deploy
inherit luv-test

#Pointing to the source bios bits
SRC_URI = "http://biosbits.org/downloads/${BPN}-${PV}.zip  \
           file://bits-cfg.txt \
           file://luv-test-bits \
           file://luv-parser-bits \
          "

SRC_URI[md5sum] = "49c88c1789686b7f387cd5bba3f4b428"
SRC_URI[sha256sum] = "a3b3cd4462384893dcb8692b3e87d60739882d247c5550120487e48ea8d4f3db"

S = "${WORKDIR}/bits-1084"

EXTRA_OECONF = "--disable-manpages --with-efi-includedir=${STAGING_INCDIR} \
                --with-efi-ldsdir=${STAGING_LIBDIR} \
                --with-efi-libdir=${STAGING_LIBDIR}"


COMPATIBLE_HOST = '(x86_64.*|i.86.*)-(linux|freebsd.*)'

# Determine the target arch for the bios modules before the native class
# clobbers TARGET_ARCH.

ORIG_TARGET_ARCH := "${TARGET_ARCH}"
python __anonymous () {
    import re
    target = d.getVar('ORIG_TARGET_ARCH', True)
    if target == "x86_64":
        bitstarget = 'x86_64'
    elif re.match('i.86', target):
        bitstarget = 'i386'
    else:
        raise bb.parse.SkipPackage("bios-efi is incompatible with target %s" % target)

}

LUV_TEST_LOG_PARSER = "luv-parser-bits"

do_configure_prepend() {
    # Return control to the main bootloader once complete.
    echo "exit" >> ${S}/boot/cfg/init.cfg
}

do_install() {
    install -d ${D}/${bindir}
    install -m 0755 ${WORKDIR}/luv-test-bits ${D}/${bindir}/bits
}

do_deploy() {

       install -d ${DEPLOYDIR}/bits

       cp -r ${B}/boot/ ${DEPLOYDIR}/bits/
       cp ${WORKDIR}/bits-cfg.txt ${DEPLOYDIR}/bits/boot/
       cp -r ${B}/efi ${DEPLOYDIR}/bits/
}


addtask deploy before do_build after do_compile

do_populate_sysroot[noexec] = "1"
