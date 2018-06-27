require luv-image.inc

DEPENDS = "python-native parted-native"
DEPENDS_append_x86 += " grub-efi"
DEPENDS_append_x86-64 += " grub-efi"

HDDIMG_ID = "423cc2c8"
LABELS_LIVE = "luv"

MACHINE_FEATURES += "efi"

CMDLINE = "${CMDLINE_BASE}"

GRUB_TIMEOUT = "2"

inherit image-live

S = "${WORKDIR}"

build_img() {

    if [ "$1" = "mbr" ]; then
        IMG="${DEPLOY_DIR_IMAGE}/${PN}-mbr.img"
    else
        IMG="${DEPLOY_DIR_IMAGE}/${PN}-gpt.img"
    fi

    VFAT="${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.hddimg"

    # Parameters of the vfat partition for test results
    # Sectors: 512 bytes
    # MiB: 1024 * 1024 bytes
    BYTES_PER_SECTOR=512
    MiB=$(expr 1024 \* 1024)
    VFAT_RESULTS=${DEPLOY_DIR_IMAGE}/${PN}-results.hddimg
    # 16MB of space to store test results
    VFAT_RESULTS_SPACE_MiB=16
    VFAT_RESULTS_SPACE=$(expr $VFAT_RESULTS_SPACE_MiB \* $MiB)
    VFAT_RESULTS_BLOCKS=$(expr $VFAT_RESULTS_SPACE / 1024)
    # TODO: do we need to dynamically generate the UUID?
    # For now, every time this UUID changes, the file etc/init.d/luv-test-manager
    # needs to be updated accordingly.
    VFAT_RESULTS_UUID=05D61523
    VFAT_RESULTS_LABEL="LUV-RESULTS"
    # Extra space at the front and rear of luv-live-image.img
    EXTRA_SPACE_MiB=2
    NUM_OF_PARTITIONS=2

    if [ -e ${VFAT_RESULTS} ]; then
        rm ${VFAT_RESULTS}
    fi

    mkdosfs -C ${VFAT_RESULTS} -S ${BYTES_PER_SECTOR} -i ${VFAT_RESULTS_UUID} \
            -n ${VFAT_RESULTS_LABEL} $VFAT_RESULTS_BLOCKS

    dd if=/dev/zero of=${IMG} bs=${BYTES_PER_SECTOR} count=1

    # Now that we are calculating sizes in MiB make sure that the value
    # is ceiled (rounded to nearest upper bound integer)
    VFAT_SIZE=$(du -L -h --apparent-size $VFAT | cut -f 1 | tr -dc '0-9.')
    VFAT_SIZE_CEILED=$(python -c "from math import ceil; print ceil($VFAT_SIZE)")
    VFAT_SIZE_MiB=${VFAT_SIZE_CEILED%.*}
    VFAT_RESULTS_SIZE_MiB=$(du -L -h --apparent-size $VFAT_RESULTS \
                            | cut -f 1 | tr -dc '0-9.')

    IMG_SIZE_MiB=$(expr $VFAT_RESULTS_SIZE_MiB + $VFAT_SIZE_MiB + $EXTRA_SPACE_MiB)

    dd if=/dev/zero of=${IMG} bs=1 seek=${IMG_SIZE_MiB}MiB count=0

    # Let "parted" tool take care of any alignment issues, if any arises
    # Support both MBR type and GPT type images
    if [ "$1" = "mbr" ]; then
        parted --align optimal ${IMG} mklabel msdos
    else
        parted --align optimal ${IMG} mklabel gpt
    fi

    # even though MBR occupies only 512 bytes we start
    # first partition after 1MiB because the default size that
    # parted tool uses is MiB and using this will inherently take
    # care of alignment issues, if any arises
    parted --align optimal ${IMG} mkpart primary fat32 1MiB \
                           "$(expr 1 + $VFAT_RESULTS_SIZE_MiB)MiB"

    # start second partition on the first sector after the first partition
    parted --align optimal ${IMG} mkpart primary fat32 \
                           "$(expr 1 + $VFAT_RESULTS_SIZE_MiB)MiB" \
                           "$(expr 1 + $VFAT_RESULTS_SIZE_MiB + $VFAT_SIZE_MiB)MiB"

    # copy "LUV-RESULTS" file into first partition
    SECTOR=$(fdisk -l ${IMG} | tail -${NUM_OF_PARTITIONS} | awk 'NR==1{print $2}')
    dd conv=notrunc if=${VFAT_RESULTS} of=${IMG} seek=1 \
                    obs=$(expr $SECTOR \* $BYTES_PER_SECTOR)

    # copy "LUV_BOOT" file into second partition without truncating output file
    SECTOR=$(fdisk -l ${IMG} | tail -${NUM_OF_PARTITIONS} | awk 'NR==2{print $2}')
    dd conv=notrunc if=${VFAT} of=${IMG} seek=1 \
                    obs=$(expr $SECTOR \* $BYTES_PER_SECTOR)

    # mark second partition as boot partition only after calculating
    # starting sector of each partition (which is already done above)
    # because fdisk will list partitions differently for MBR and GPT.
    # Also mark second partition as boot partition only for mbr type
    # image because some distros do not automatically mount
    # "EFI System Partition"

    parted ${IMG} set 1 boot off
    if [ "$1" = "mbr" ]; then
        parted ${IMG} set 2 boot on
    fi

}

python do_bootimg_prepend() {
    bb.build.exec_func('build_luv_cfg', d)
}

build_imgs() {
    build_img mbr
    build_img gpt
}

python do_create_img() {
    bb.build.exec_func('build_imgs', d)
}

do_bootimg[depends] += "${INITRD_IMAGE_LIVE}:do_build"
do_bootimg[depends] += "virtual/kernel:do_populate_sysroot"
do_bootimg[depends] += "shim-signed:do_deploy"
do_create_img[fakeroot]= "1"

addtask create_img after do_bootimg before do_build
addtask do_unpack before do_build
addtask image_ext4 before do_bootimg before do_build
