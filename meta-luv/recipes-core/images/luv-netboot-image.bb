require luv-image.inc

# bootimg.bbclass will set PCBIOS="1" if efi is not found in the MACHINE_FEATURES. For
# netboot we don't include 'efi' as a machine feature; this would imply the creation
# efi-related artifacts. We don't need them because this image is intended to be embedded
# into the grub image.
PCBIOS_remove = "1"
PCBIOS_append = "0"

CMDLINE = "${CMDLINE_BASE} luv.netboot"

GRUB_EFI_LOADER_IMAGE_x86-64 = "grub-efi-bootx64.efi"
GRUB_EFI_LOADER_IMAGE_x86 = "grub-efi-bootia32.efi"
GRUB_EFI_LOADER_IMAGE_aarch64 = "grub-efi-bootaa64.efi"

DEST_EFI_LOADER_IMAGE_x86-64 = "bootx64.efi"
DEST_EFI_LOADER_IMAGE_x86 = "bootia32.efi"
DEST_EFI_LOADER_IMAGE_aarch64 = "bootaa64.efi"

inherit luv-efi
inherit image-live
inherit deploy

do_mkimage[depends] += "dosfstools-native:do_populate_sysroot \
                        mtools-native:do_populate_sysroot \
                        cdrtools-native:do_populate_sysroot \
                        virtual/kernel:do_deploy \
			${_BITSDEPENDS}"

do_bootimg[noexec] = "1"

do_populate_image() {
	install -d ${HDDDIR}${EFIDIR}
	install -m 0644 ${GRUBCFG} ${HDDDIR}${EFIDIR}
	install -m 0644 ${LUV_CFG} ${HDDDIR}
	if [ "${TARGET_ARCH}" != "aarch64" ]; then
		efi_populate_bits ${HDDDIR}
	fi
	build_hddimg
}

python do_mkimage() {
    set_live_vm_vars(d, 'LIVE')
    bb.build.exec_func('build_efi_cfg', d)
    bb.build.exec_func('build_luv_cfg', d)
    bb.build.exec_func('do_populate_image', d)
    bb.build.exec_func('create_symlinks', d)
}

do_deploy() {
	rm -f ${DEPLOY_DIR_IMAGE}/${PN}.efi
	if [ "${TARGET_ARCH}" = "aarch64" ]; then
		ln -s ${DEPLOY_DIR_IMAGE}/${GRUB_EFI_LOADER_IMAGE} ${DEPLOY_DIR_IMAGE}/${PN}.efi
	else
		ln -s ${DEPLOY_DIR_IMAGE}/${GRUB_EFI_LOADER_IMAGE} ${DEPLOY_DIR_IMAGE}/${PN}.efi
	fi
}

addtask do_mkimage before do_build
addtask do_deploy before do_build after do_mkimage
addtask image_ext4 before do_bootimg before do_build

do_mkimage[depends] += "${INITRD_IMAGE_LIVE}:do_build"
do_deploy[depends] += "${MLPREFIX}grub-efi:do_deploy"
