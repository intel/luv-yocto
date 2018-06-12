FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

# Parameters used only when building for netboot.
# There should be a way to obtain this programmatically. However, this is the way
# in which other recipes specify the ramdisk.
GRUB_MEMDISK_IMAGE = "luv-netboot-image"
GRUB_MEMDISK="${DEPLOY_DIR_IMAGE}/${GRUB_MEMDISK_IMAGE}-${MACHINE}.hddimg"

# Fixup the GRUB_BUIIDIN variable to include both the memdisk module as well as the
# disk image.
python __anonymous(){
    if bb.utils.contains('DISTRO_FEATURES', 'luv-netboot', True, False, d):
        d.appendVar("GRUB_BUILDIN", ' memdisk -m ${GRUB_MEMDISK}')
}

COMMON_GRUB_x86 += "file://0001-pe32.h-add-header-structures-for-TE-and-DOS-executab.patch \
           file://0002-shim-add-needed-data-structures.patch \
           file://0003-efi-chainloader-implement-an-UEFI-Exit-service-for-s.patch \
           file://0004-efi-chainloader-port-shim-to-grub.patch \
           file://0005-efi-chainloader-use-shim-to-load-and-verify-an-image.patch \
           file://0006-efi-chainloader-boot-the-image-using-shim.patch \
           file://0007-efi-chainloader-take-care-of-unload-undershim.patch \
           file://0008-grub-shim-Add-call-wrappers-for-32-bit-systems.patch \
           file://0009-grub-shim-compile-chainloader.c-for-32bit-system.patch \
           file://0010-grub-core-efi-chainloader-verify-and-load-image-sepa.patch \
           file://0011-grub-core-loader-extend-loader-options.patch \
           file://0012-grub-core-loader-add-support-for-memdisks.patch \
           file://0013-grub-core-loader-pass-current-disk-device-to-loaded-.patch \
           file://0001-tsc-Change-default-tsc-calibration-method-to-pmtimer.patch \
           file://0014-grub-core-Make-user-aware-of-exit_boot_service-failure.patch \
          "

# Add grub-efi patches for x86/x86-64/arm64
SRC_URI_append_x86 = "${COMMON_GRUB_x86} \
                     "
SRC_URI_append_x86-64 = "${COMMON_GRUB_x86} \
                        "
 
SRC_URI += "file://0014-grub-util-set-prefix-to-EFI-BOOT-when-booting-f.patch \
          "

GRUB_BUILDIN = "boot linux ext2 fat serial part_msdos part_gpt \
        normal efi_gop iso9660 configfile search efinet tftp all_video chain \
        gfxmenu jpeg gfxterm"
do_deploy[depends] += "${@bb.utils.contains('DISTRO_FEATURES', 'luv-netboot','luv-netboot-image:do_mkimage' , '', d)}"

