require grub2.inc

DEPENDS += "autogen-native"

DEFAULT_PREFERENCE = "-1"
DEFAULT_PREFERENCE_arm = "1"

PV = "2.00+${SRCPV}"
SRCREV = "87de66d9d83446ecddb29cfbdf7369102c8e209e"
SRC_URI = "git://git.savannah.gnu.org/grub.git \
           file://cfg \
           file://grub-2.00-fpmath-sse-387-fix.patch \
           file://autogen.sh-exclude-pc.patch \
           file://grub-2.00-add-oe-kernel.patch \
           file://0001-Fix-build-with-glibc-2.20.patch \
          "

S = "${WORKDIR}/git"

COMPATIBLE_HOST = '(x86_64.*|i.86.*|arm.*|aarch64.*)-(linux.*|freebsd.*)'

inherit autotools-brokensep gettext texinfo deploy

PACKAGECONFIG ??= ""
PACKAGECONFIG[grub-mount] = "--enable-grub-mount,--disable-grub-mount,fuse"
PACKAGECONFIG[device-mapper] = "--enable-device-mapper,--disable-device-mapper,lvm2"

# configure.ac has code to set this automagically from the target tuple
# but the OE freeform one (core2-foo-bar-linux) don't work with that.

GRUBPLATFORM_arm = "uboot"
GRUBPLATFORM_aarch64 = "efi"
GRUBPLATFORM ??= "pc"

EXTRA_OECONF = "--with-platform=${GRUBPLATFORM} --disable-grub-mkfont --program-prefix="" \
                --enable-liblzma=no --enable-device-mapper=no --enable-libzfs=no"

export ac_cv_path_HELP2MAN=""

do_configure_prepend() {
      ./autogen.sh
}

do_install_append_class-native() {
        install -m 755 -D grub-mkimage ${D}${bindir}
}

do_install_append_class-target() {
    # Search for the grub.cfg on the local boot media by using the
    # built in cfg file provided via this recipe
    grub-mkimage -c ../cfg -p /EFI/BOOT -d ./grub-core/ \
        -O ${GRUB_TARGET}-${GRUBPLATFORM} -o ./${GRUB_IMAGE} \
        boot linux ext2 fat serial part_msdos part_gpt \
        normal efi_gop iso9660 search efinet tftp all_video chain \
        gfxmenu jpeg gfxterm

    install -m 0755 -D ${B}/${GRUB_IMAGE} ${D}${bindir}
}

GRUB_TARGET_aarch64 = "arm64"
GRUB_IMAGE_aarch64 = "grubaa64.efi"

do_mkimage_class-native() {
        :
}

do_deploy() {
    install -m 0755 -D ${B}/${GRUB_IMAGE} ${DEPLOYDIR}
}

do_deploy_class-native() {
        :
}

addtask deploy after do_install before do_build

# debugedit chokes on bare metal binaries
INHIBIT_PACKAGE_DEBUG_SPLIT = "1"

RDEPENDS_${PN}_class-target = "diffutils freetype"
FILES_${PN}-dbg += "${libdir}/${BPN}/*/.debug"

INSANE_SKIP_${PN} = "arch"
INSANE_SKIP_${PN}-dbg = "arch"

DEPENDS_class-target += "grub-native"
BBCLASSEXTEND = "native"
