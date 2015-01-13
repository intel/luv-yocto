LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/LICENSE;md5=4d92cd373abda3937c2bc47fbc49d690"

DEPENDS_${PN} = "grub-efi bits"

HDDDIR = "${S}/hddimg"
HDDIMG_ID = "423cc2c8"
LABELS = "luv"

INITRD_IMAGE = "core-image-efi-initramfs"
INITRD = "${DEPLOY_DIR_IMAGE}/${INITRD_IMAGE}-${MACHINE}.cpio.gz"
MACHINE_FEATURES += "efi"
APPEND = "crashkernel=256M console=ttyS0,115200 console=ttyPCH0,115200"

SPLASH_IMAGE = "blue-luv.jpg"

GRUB_TIMEOUT = "2"

inherit bootimg

SRC_URI = "file://blue-luv.jpg"

build_img() {
    IMG="${DEPLOY_DIR_IMAGE}/${PN}.img"
    VFAT="${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.hddimg"

    # Parameters of the vfat partition for test results
    # Sectors: 512 bytes
    # Blocks: 1024 bytes
    VFAT_RESULTS=${DEPLOY_DIR_IMAGE}/${PN}-results.hddimg
    # 8MB of space for test results
    VFAT_RESULTS_SPACE=8388608
    VFAT_RESULTS_BLOCKS=$(expr $VFAT_RESULTS_SPACE / 1024)
    # TODO: do we need to dynamically generate the UUID?
    # For now, every time this UUID changes, the file etc/init.d/luv-test-manager
    # needs to be updated accordingly.
    VFAT_RESULTS_UUID=05d61523
    VFAT_RESULTS_LABEL="luv-results"

    mkdosfs -C ${VFAT_RESULTS} -S 512 -i ${VFAT_RESULTS_UUID} \
            -n ${VFAT_RESULTS_LABEL} $VFAT_RESULTS_BLOCKS

    dd if=/dev/zero of=$IMG bs=512 count=1

    VFAT_SIZE=$(du -L --apparent-size -bs $VFAT | cut -f 1)
    VFAT_RESULTS_SIZE=$(du -L --apparent-size -bs $VFAT_RESULTS | cut -f 1)

    IMG_SIZE=$(expr $VFAT_SIZE + $VFAT_RESULTS_SIZE + 512)

    dd if=/dev/zero of=$IMG bs=1 seek=$IMG_SIZE count=0

    parted $IMG mklabel msdos

    parted $IMG mkpart primary fat32 0% "${VFAT_RESULTS_SIZE}B"

    # start second partition on the first sector after the first partition
    parted $IMG mkpart primary fat32 "$(expr $VFAT_RESULTS_SIZE + 512)B" \
           "$(expr $VFAT_SIZE + $VFAT_RESULTS_SIZE)B"

    parted $IMG set 2 boot on

    dd conv=notrunc if=${VFAT_RESULTS} of=$IMG seek=1 bs=512
    dd if=${VFAT} of=$IMG seek=$(expr $(expr $VFAT_RESULTS_SIZE / 512) + 1) bs=512

}

python do_create_img() {
    bb.build.exec_func('build_img', d)
}

do_bootimg[depends] += "${INITRD_IMAGE}:do_rootfs"
do_bootimg[depends] += "virtual/kernel:do_populate_sysroot"

addtask create_img after do_bootimg before do_build
addtask do_unpack before do_build
