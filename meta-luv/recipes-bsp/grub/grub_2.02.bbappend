FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += " file://cfg \
	"

inherit autotools-brokensep gettext texinfo deploy

export ac_cv_path_HELP2MAN=""

GRUB_TARGET_aarch64 = "arm64"
GRUB_IMAGE_aarch64 = "grub-efi-bootaa64.efi"
DEPENDS_append_class-target = " grub-native "
RDEPENDS_${PN}_class-target = "diffutils freetype"

do_install_append_class-native() {
         install -m 755 -D grub-mkimage ${D}${bindir}
}

do_install_append_class-target() {
     # Search for the grub.cfg on the local boot media by using the
     # built in cfg file provided via this recipe
     grub-mkimage -c ../cfg -p /EFI/BOOT -d ./grub-core/ \
         -O ${GRUB_TARGET}-${GRUBPLATFORM} -o ./${GRUB_IMAGE} \
         boot linux ext2 fat serial part_msdos part_gpt \
         normal efi_gop iso9660 configfile search efinet tftp all_video chain \
         gfxmenu jpeg gfxterm
     install -m 0755 -D ${B}/${GRUB_IMAGE} ${D}${bindir}
}

do_deploy() {
    install -m 0755 -D ${B}/${GRUB_IMAGE} ${DEPLOYDIR}
}

do_deploy_class-native() {
        :
}

addtask deploy after do_install before do_build

BBCLASSEXTEND = "native"

RDEPENDS_${PN} = " freetype "
PACKAGES = "grub-dbg grub-staticdev grub-dev grub-doc grub-locale grub"
FILES_grub-editenv = ""
