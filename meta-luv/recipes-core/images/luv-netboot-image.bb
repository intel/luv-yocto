LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=4d92cd373abda3937c2bc47fbc49d690"

DEPENDS_${PN} = "bits"

INITRD_IMAGE = "core-image-efi-initramfs"
INITRD = "${DEPLOY_DIR_IMAGE}/${INITRD_IMAGE}-${MACHINE}.cpio.gz"

# bootimg.bbclass will set PCBIOS="1" if efi is not found in the MACHINE_FEATURES. For
# netboot we don't include 'efi' as a machine feature; this would imply the creation
# efi-related artifacts. We don't need them because this image is intended to be embedded
# into the grub image.
PCBIOS_remove = "1"
PCBIOS_append = "0"

APPEND = "debug crashkernel=256M console=ttyS0,115200 console=ttyPCH0,115200 luv.netboot ip=dhcp log_buf_len=1M"
APPEND_netconsole = "luv_netconsole=10.11.12.13,64001"
APPEND_aarch64 = "crashkernel=256M console=ttyAMA0 uefi_debug acpi=force luv.netboot"

HDDDIR = "${S}/hddimg"

inherit luv-efi
inherit bootimg
inherit deploy

# reuse the same splash screen as in the disk live image
FILESEXTRAPATHS_append := "${THISDIR}/luv-live-image:"
SPLASH_IMAGE = "blue-luv.jpg"
SRC_URI = "file://blue-luv.jpg"

do_mkimage[depends] += "${EXTRABOOTIMGDEPS} \
                        dosfstools-native:do_populate_sysroot \
                        mtools-native:do_populate_sysroot \
                        cdrtools-native:do_populate_sysroot \
                        virtual/kernel:do_deploy \
			${_BITSDEPENDS}"

do_bootimg[noexec] = "1"

do_populate_image() {
	install -d  ${HDDDIR}${EFIDIR}
	if [ "${TARGET_ARCH}" != "aarch64" ]; then
		efi_populate_bits ${HDDDIR}
	fi
	install -m 0644 ${GRUBCFG} ${HDDDIR}${EFIDIR}
	build_hddimg
}

python do_mkimage() {
    bb.build.exec_func('build_efi_cfg', d)
    bb.build.exec_func('do_populate_image', d)
    bb.build.exec_func('create_symlinks', d)
}

do_deploy() {
	rm -f ${DEPLOY_DIR_IMAGE}/${PN}.efi
	if [ "${TARGET_ARCH}" == "aarch64" ]; then
		ln -s ${DEPLOY_DIR_IMAGE}/bootaa64.efi ${DEPLOY_DIR_IMAGE}/${PN}.efi
	else
		ln -s ${DEPLOY_DIR_IMAGE}/bootx64.efi ${DEPLOY_DIR_IMAGE}/${PN}.efi
	fi
}

addtask do_mkimage before do_build
addtask do_deploy before do_build after do_mkimage

do_mkimage[depends] += "${INITRD_IMAGE}:do_build"
do_deploy[depends] += "${_RDEPENDS}:do_deploy"

