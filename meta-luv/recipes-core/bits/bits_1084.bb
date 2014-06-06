
SUMMARY = "BIOSBits is a test suite that runs UEFI BIOS tests."

DESCRIPTION = " The Intel BIOS Implementation Test Suite (BITS) provides a bootable \
pre-OS environment for testing BIOSes and in particular their initialization \
of Intel processors, hardware, and technologies. BITS can verify your BIOS \
against many Intel recommendations. In addition, BITS includes Intel's \
official reference code as provided to BIOS, which you can use to override \
your BIOS's hardware initialization with a known-good configuration, and then boot an OS."


HOMEPAGE = "http://biosbits.org/"
SECTION = "bootloaders"

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=49c88c1789686b7f387cd5bba3f4b428"


DEPENDS = "autogen-native flex-native"
RDEPENDS_${PN} = "diffutils freetype xz"
PR = "r1"
PV = "1084"

SRC_URI = "http://biosbits.org/downloads/bits-${PV}.zip \
           file://bits-cfg.txt \
          "

SRC_URI[md5sum] = "49c88c1789686b7f387cd5bba3f4b428"
SRC_URI[sha256sum] = "a3b3cd4462384893dcb8692b3e87d60739882d247c5550120487e48ea8d4f3db"

COMPATIBLE_HOST = '(x86_64.*|i.86.*)-(linux|freebsd.*)'

# Determine the target arch for the bios bits modules before the test
# clobbers TARGET_ARCH.
ORIG_TARGET_ARCH := "${TARGET_ARCH}"
python __anonymous () {
    import re
    target = d.getVar('ORIG_TARGET_ARCH', True)
    if target == "x86_64":
        biostarget = 'x86_64'
        biosimage = "bootx64.efi"
    elif re.match('i.86', target):
        biostarget = 'i386'
        biosimage = "bootia32.efi"
    else:
        raise bb.parse.SkipPackage("Bios Bits is incompatible with target %s" % target)
}

inherit autotools
inherit gettext
inherit native
inherit deploy

do_deploy() {
    install -m 644 ${B}/bits-cfg.txt ${DEPLOYDIR}/bits-${PV}/boot/bits-cfg.txt
}
addtask deploy after do_install before do_build

#Installing is nothing but putting things in place
do_install() {
    # Creating a directory
    install -d ${D}${DEPLOYDIR}/bits-${PV}
}

