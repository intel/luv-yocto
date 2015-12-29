FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://0001-pe32.h-add-header-structures-for-TE-and-DOS-executab.patch \
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
          "

GRUB_BUILDIN = "boot linux ext2 fat serial part_msdos part_gpt \
        normal efi_gop iso9660 search efinet tftp all_video chain \
        gfxmenu jpeg gfxterm"

