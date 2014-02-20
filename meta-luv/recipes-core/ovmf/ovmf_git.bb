DESCRIPTION = "OVMF - UEFI firmware for Qemu and KVM"
HOMEPAGE = "http://sourceforge.net/apps/mediawiki/tianocore/index.php?title=OVMF"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://OvmfPkg/License.txt;md5=ffd52cf9a8e0e036b9a61a0de2dc87ed"

SRC_URI = "git://github.com/tianocore/edk2.git;branch=master \
	file://0001-BaseTools-Don-t-override-CFLAGS.patch \
	file://0002-BaseTools-Don-t-override-LFLAGS.patch"

SRCREV="${AUTOREV}"

inherit native
BBCLASSEXTEND = "native nativesdk"

S = "${WORKDIR}/git"

# OVMF has trouble building with the default optimization of -O2.
BUILD_OPTIMIZATION="-pipe"

do_compile() {
    export LFLAGS="${LDFLAGS}"
    ${S}/OvmfPkg/build.sh -a X64 -b RELEASE
}

do_install() {
    install -d ${D}${datadir}/ovmf
    install -m 0755 `find ${S}/Build -name OVMF.fd` \
	${D}${datadir}/ovmf/bios.bin
}
