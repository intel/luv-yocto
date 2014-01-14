DESCRIPTION = "OVMF - UEFI firmware for Qemu and KVM"
HOMEPAGE = "http://sourceforge.net/apps/mediawiki/tianocore/index.php?title=OVMF"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://OvmfPkg/License.txt;md5=ffd52cf9a8e0e036b9a61a0de2dc87ed"

#SRC_URI = "git://github.com/tianocore/edk2.git;protocol=efi;nocheckout=1"
SRC_URI = "git:///home/matt/src/3rd-party/efi/edk2.git;protocol=file"
SRCREV="${AUTOREV}"

BBCLASSEXTEND = "native nativesdk"

DEPENDS_class-native += "qemu-native"

S = "${WORKDIR}/git"

do_compile() {
    ${S}/OvmfPkg/build.sh -a X64 -b RELEASE
}

do_install() {
    install -d ${D}${datadir}/ovmf
    install -m 0755 ${S}/Build/OvmfX64/DEBUG_GCC47/FV/OVMF.fd \
	${D}${datadir}/ovmf/bios.bin
}
