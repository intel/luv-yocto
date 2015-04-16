DESCRIPTION = "Linaro LEG ACPI kernel build recipe for Aarch64"
SECTION = "kernel"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://COPYING;md5=d7810fab7487fb0aad327b76f1be7cd7"

DEPENDS += "xz-native bc-native" 
DEPENDS_aarch64 += "libgcc"

inherit kernel siteinfo deploy

SRCREV = "${AUTOREV}"
PV = "3.18+git${SRCREV}"

SRC_URI = "git://git.linaro.org/leg/acpi/leg-kernel.git;protocol=git;branch=leg-kernel file://defconfig"

S = "${WORKDIR}/git"

COMPATIBLE_MACHINE = "genericarmv8"
KERNEL_IMAGETYPE = "Image"

BOOTARGS_COMMON = "crashkernel=256M console=ttyAMA0 uefi_debug acpi=force"

SRC_URI[md5sum] = "730923a8ee0ded0ddbfe91e500cb3706"
SRC_URI[sha256sum] = "53c64f40cce74531d117cd3495b5c0c52b24e1ad1b7de9fa27deea396edb4d87"
