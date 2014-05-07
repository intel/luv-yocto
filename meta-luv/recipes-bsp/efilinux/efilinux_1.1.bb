SUMMARY = "A small BSD-licensed reference UEFI OS loader"
SECTION = "bootloaders"

LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://README;md5=6b473c0ed2b77b2aecb3251a67a236e1"

SRC_URI = "https://www.kernel.org/pub/linux/utils/boot/efilinux/efilinux-${PV}.tar.gz"

SRC_URI[md5sum] = "07de903ff6c6b2916ecad091f7be9e2a"
SRC_URI[sha256sum] = "fb294a78acfed1fbd48a1f197a53d263049eaf8de40399fbc821d36dca9fd4cb"

DEPENDS = "gnu-efi"
inherit deploy

def gnu_efi_arch(d):
    import re
    tarch = d.getVar("TARGET_ARCH", True)
    if re.match("i[3456789]86", tarch):
        return "ia32"
    return tarch

EXTRA_OEMAKE = "'ARCH=${@gnu_efi_arch(d)}' 'LIBDIR=${STAGING_LIBDIR}' 'INCDIR=${STAGING_INCDIR}'"

do_deploy () {
        install ${B}/efilinux.efi ${DEPLOYDIR}
}
addtask deploy before do_build after do_compile
