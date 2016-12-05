# initramfs image for the EFI test suite
DESCRIPTION = "Small image capable of booting a device and running the suite of \
EFI tests."

IMAGE_INSTALL = "\
    base-files base-passwd udev systemd \
    keymaps \
    kernel-image bash coreutils grep util-linux-agetty \
    util-linux-mount util-linux-umount kmod \
    shadow util-linux procps \
    plymouth plymouth-set-default-theme \
    "

X86_ADDITIONS = ""

IMAGE_INSTALL_append_qemux86 = "${X86_ADDITIONS} umip-tests"
IMAGE_INSTALL_append_qemux86-64 = "${X86_ADDITIONS} umip-tests lib32-umip-tests"

export IMAGE_BASENAME = "core-image-efi-initramfs"

IMAGE_LINGUAS = ""

LICENSE = "MIT"

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"

USE_DEVFS = "0"
inherit core-image

IMAGE_ROOTFS_SIZE = "8192"

BAD_RECOMMENDATIONS += "busybox-syslog"


