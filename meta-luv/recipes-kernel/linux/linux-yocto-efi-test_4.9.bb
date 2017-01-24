# linux-yocto-custom.bb:
#
#   An example kernel recipe that uses the linux-yocto and oe-core
#   kernel classes to apply a subset of yocto kernel management to git
#   managed kernel repositories.
#
#   To use linux-yocto-custom in your layer, create a
#   linux-yocto-custom.bbappend file containing at least the following
#   lines:
#
#     FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"
#     COMPATIBLE_MACHINE_yourmachine = "yourmachine"
#
#   You must also provide a Linux kernel configuration. The most direct
#   method is to copy your .config to files/defconfig in your layer,
#   in the same directory as the bbappend and add file://defconfig to
#   your SRC_URI.
#
#   To use the yocto kernel tooling to generate a BSP configuration
#   using modular configuration fragments, see the yocto-bsp and
#   yocto-kernel tools documentation.
#
# Warning:
#
#   Building this example without providing a defconfig or BSP
#   configuration will result in build or boot errors. This is not a
#   bug.
#
#
# Notes:
#
#   patches: patches can be merged into to the source git tree itself,
#            added via the SRC_URI, or controlled via a BSP
#            configuration.
#
#   defconfig: When a defconfig is provided, the linux-yocto configuration
#              uses the filename as a trigger to use a 'allnoconfig' baseline
#              before merging the defconfig into the build.
#
#              If the defconfig file was created with make_savedefconfig,
#              not all options are specified, and should be restored with their
#              defaults, not set to 'n'. To properly expand a defconfig like
#              this, specify: KCONFIG_MODE="--alldefconfig" in the kernel
#              recipe.
#
#   example configuration addition:
#            SRC_URI += "file://smp.cfg"
#   example patch addition (for kernel v3.4 only):
#            SRC_URI += "file://0001-linux-version-tweak.patch
#   example feature addition (for kernel v3.4 only):
#            SRC_URI += "file://feature.scc"
#

KBRANCH="master"
inherit kernel
require recipes-kernel/linux/linux-yocto.inc

KERNEL_FEATURES_remove= " features/debug/printk.scc"

# Override SRC_URI in a bbappend file to point at a different source
# tree if you do not want to build from Linus' tree.
SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git;protocol=git;branch=${KBRANCH};name=machine"

# Fix buggy firmware + kernel v4.9 + "Detect illegal access to UEFI Boot Services memory regions"
SRC_URI += "file://0001-x86-efi-Install-new_memmap-at-the-end-of-efi_late_in.patch"

# Detect illegal access to UEFI Boot Services memory regions.
SRC_URI += "file://0001-Add-function-to-fixup-page-faults-in-BOOT_SERVICES_-.patch \
            file://0002-efi-allow-efi_mem_desc_lookup-find-EFI_BOOT_SERVICES.patch \
            file://0003-x86-efi-Fixup-faults-from-UEFI-firmware.patch \
            file://0004-x86-efi-Introduce-EFI_BOOT_SERVICES_WARN.patch \
            file://0001-selftests-x86-Add-tests-for-User-Mode-Instruction-Pr.patch \
            file://0002-selftests-x86-Add-more-tests-for-User-Mode-Instructi.patch \
           "
# These patches are under discussion on ML
SRC_URI += "file://0001-serial-SPCR-check-bit-width-for-the-16550-UART.patch  \
            file://0001-serial-pl011-add-console-matching-function.patch \
           "

COMMON_CFG_x86 = " file://${MACHINE}/defconfig \
                   file://qemux86/modules.cfg \
                   file://qemux86/display.cfg \
                   file://qemux86/ram_block.cfg \
                   file://qemux86/debug.cfg \
                   file://qemux86/efi.cfg \
                   file://qemux86/usb_hcd.cfg \
                   file://qemux86/network.cfg \
                   file://qemux86/network-devices.cfg \
                   file://qemux86/linux_quirks.cfg \
                   file://qemux86/usb_ethernet.cfg \
                 "

# Add the kernel configuration fragments for x86/x86-64/arm64
SRC_URI_append_x86 = "${COMMON_CFG_x86}"
SRC_URI_append_x86-64 = "${COMMON_CFG_x86} \
                         file://qemux86/ndctl.cfg \
                        "
SRC_URI_append_aarch64 = " file://qemuarm64/defconfig \
                           file://qemuarm64/network.cfg \
                         "

# Override KCONFIG_MODE to '--alldefconfig' from the default '--allnoconfig'
KCONFIG_MODE = '--alldefconfig'
LINUX_VERSION ?= "4.9"
LINUX_VERSION_EXTENSION ?= "-efitest"

# Override SRCREV to point to a different commit in a bbappend file to
# build a different release of the Linux kernel.
# tag: v4.9 69973b830859bc6529a7a0468ba0d80ee5117826
SRCREV = "69973b830859bc6529a7a0468ba0d80ee5117826"

PR = "r5"
PV = "${LINUX_VERSION}+git${SRCPV}"

# Override COMPATIBLE_MACHINE to include your machine in a bbappend
# file. Leaving it empty here ensures an early explicit build failure.
COMPATIBLE_MACHINE = "qemux86|qemux86-64|qemuarm64"

do_install_append() {
    if [ "${TARGET_ARCH}" = "x86_64" ]; then
         # There are 2 copies of the NVDIMM modules which are built. This is a
         # temporary fix to make sure the correct set of modules are used.
         rm -rf ${D}/lib/modules/${LINUX_VERSION}.0-yocto-standard/kernel/drivers/nvdimm/
         rm ${D}/lib/modules/${LINUX_VERSION}.0-yocto-standard/kernel/drivers/acpi/nfit/nfit.ko
    fi
}
