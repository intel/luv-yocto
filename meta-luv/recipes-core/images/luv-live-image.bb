LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=3f40d7994397109285ec7b81fdeb3b58"

DEPENDS_${PN} = "grub-efi"

HDDDIR = "${S}/hddimg"
LABELS = "luv"

INITRD_IMAGE = "core-image-efi-initramfs"
INITRD = "${DEPLOY_DIR_IMAGE}/${INITRD_IMAGE}-${MACHINE}.cpio.gz"
MACHINE_FEATURES += "efi"

inherit bootimg

build_img() {
    IMG="${DEPLOY_DIR_IMAGE}/${PN}.img"
    VFAT="${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.hddimg"

    dd if=/dev/zero of=$IMG bs=512 count=1

    VFAT_SIZE=$(du -L --apparent-size -bs $VFAT | cut -f 1)

    IMG_SIZE=$(expr $VFAT_SIZE + 512)

    dd if=/dev/zero of=$IMG bs=1 seek=$IMG_SIZE count=0

    parted $IMG mklabel msdos

    parted $IMG mkpart primary 0% "${VFAT_SIZE}B"
    parted $IMG set 1 boot on

    dd if=${VFAT} of=$IMG seek=1 bs=512
}

python do_create_img() {
    bb.build.exec_func('build_img', d)
}

do_bootimg[depends] += "${INITRD_IMAGE}:do_rootfs"
do_bootimg[depends] += "virtual/kernel:do_populate_sysroot"

addtask create_img after do_bootimg before do_build
