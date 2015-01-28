# initramfs image for the EFI test suite
DESCRIPTION = "Small image capable of booting a device and running the suite of \
EFI tests."

IMAGE_INSTALL = "\
    base-files base-passwd netbase udev sysvinit initscripts keymaps \
    kernel-image fwts bash coreutils gawk grep util-linux-agetty \
    util-linux-mount util-linux-umount kmod sed tar net-tools \
    shadow util-linux procps efivarfs-test \
    psplash kernel-efi-warnings \
    "

X86_ADDITIONS = "chipsec kexec vmcore-dmesg bits"

IMAGE_INSTALL_append_qemux86 = "${X86_ADDITIONS}"
IMAGE_INSTALL_append_qemux86-64 = "${X86_ADDITIONS}"

export IMAGE_BASENAME = "core-image-efi-initramfs"

IMAGE_LINGUAS = ""

LICENSE = "MIT"

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"

USE_DEVFS = "0"
inherit core-image

IMAGE_ROOTFS_SIZE = "8192"

BAD_RECOMMENDATIONS += "busybox-syslog"


