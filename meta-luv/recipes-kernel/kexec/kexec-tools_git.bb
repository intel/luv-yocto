# Should really try and find some way to share this.

SUMMARY = "Kexec fast reboot tools"
DESCRIPTION = "Kexec is a fast reboot feature that lets you reboot to a new Linux kernel"
AUTHOR = "Eric Biederman"
HOMEPAGE = "http://kernel.org/pub/linux/utils/kernel/kexec/"
SECTION = "kernel/userland"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=ea5bed2f60d357618ca161ad539f7c0a \
                    file://kexec/kexec.c;beginline=1;endline=20;md5=af10f6ae4a8715965e648aa687ad3e09"
DEPENDS = "zlib xz"

SRC_URI = "git://git.kernel.org/pub/scm/utils/kernel/kexec/kexec-tools.git;protocol=git"

SRCREV="${AUTOREV}"

S = "${WORKDIR}/git"

inherit autotools

#COMPATIBLE_HOST = '(x86_64.*|i.86.*|arm.*|powerpc.*|mips.*)-(linux|freebsd.*)'

PACKAGES += "kexec kdump"

FILES_kexec = "${sbindir}/kexec"
FILES_kdump = "${sbindir}/kdump"

INSANE_SKIP_${PN} = "arch"
