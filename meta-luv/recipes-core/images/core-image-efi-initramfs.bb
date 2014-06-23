# initramfs image for the EFI test suite
DESCRIPTION = "Small image capable of booting a device and running the suite of \
EFI tests."

IMAGE_INSTALL = "\
    base-files base-passwd netbase udev sysvinit initscripts keymaps \
    kernel-image fwts bash coreutils gawk grep util-linux-agetty \
    util-linux-mount util-linux-umount kmod sed tar net-tools \
    shadow util-linux procps efivarfs-test \
    psplash kexec vmcore-dmesg \
    "

export IMAGE_BASENAME = "core-image-efi-initramfs"

IMAGE_LINGUAS = ""

LICENSE = "MIT"

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"
inherit core-image

IMAGE_ROOTFS_SIZE = "8192"

BAD_RECOMMENDATIONS += "busybox-syslog"


