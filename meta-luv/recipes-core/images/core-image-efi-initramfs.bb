# initramfs image for the EFI test suite
DESCRIPTION = "Small image capable of booting a device and running the suite of \
EFI tests."

IMAGE_INSTALL = "packagegroup-core-boot kexec-tools kernel-image fwts"

export IMAGE_BASENAME = "core-image-efi-initramfs"

IMAGE_LINGUAS = ""

LICENSE = "MIT"

IMAGE_FSTYPES = "${INITRAMFS_FSTYPES}"
inherit core-image

IMAGE_ROOTFS_SIZE = "8192"

BAD_RECOMMENDATIONS += "busybox-syslog"


