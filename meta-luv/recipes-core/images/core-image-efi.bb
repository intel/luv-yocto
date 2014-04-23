DESCRIPTION = "A minimal image with essential EFI and kernel tools"

IMAGE_INSTALL = "\
    base-files base-passwd netbase udev sysvinit initscripts keymaps \
    kexec-tools kernel-image fwts bash coreutils gawk grep util-linux-agetty \
    util-linux-mount util-linux-umount kmod sed tar net-tools \
    shadow util-linux procps "

inherit core-image

IMAGE_ROOTFS_SIZE = "8192"
