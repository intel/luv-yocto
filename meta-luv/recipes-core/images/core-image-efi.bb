DESCRIPTION = "A minimal image with essential EFI and kernel tools"

IMAGE_INSTALL = "packagegroup-core-boot kexec-tools kernel-image"

inherit core-image

IMAGE_ROOTFS_SIZE = "8192"
