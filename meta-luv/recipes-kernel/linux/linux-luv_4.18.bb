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

KERNEL_FEATURES_remove= " features/debug/printk.scc \
                           features/kernel-sample/kernel-sample.scc \
                         "

# Override SRC_URI in a bbappend file to point at a different source
# tree if you do not want to build from Linus' tree.
SRC_URI = "git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux.git;protocol=git;branch=${KBRANCH};name=machine"

# These patches are under discussion on ML
SRC_URI += "file://0001-serial-SPCR-check-bit-width-for-the-16550-UART.patch  \
           "

# Detect illegal accesses to EFI regions (like EFI_CONVENTIONAL_MEMORY,
# EFI_LOADER_CODE/DATA, EFI_BOOT_SERVICES_CODE/DATA) by firmware.
SRC_URI += "file://0001-PCI-Vulcan-AHCI-PCI-bar-fix-for-Broadcom-Vulcan-earl.patch \
            file://0002-ahci-thunderx2-Fix-for-errata-that-affects-stop-engi.patch \
            file://0001-x86-efi-remove-__init-attribute-from-memory-mapping-.patch \
            file://0002-x86-efi-Save-EFI_MEMORY_MAP-passed-by-firmware-perma.patch \
            file://0003-x86-efi-Copy-support-functions-to-install-uninstall-.patch \
            file://0004-efi-Allow-efi_mem_desc_lookup-find-illegally-accesse.patch \
            file://0005-x86-efi-Add-function-to-fixup-page-faults-in-illegal.patch \
            file://0006-x86-mm-Allocate-pages-without-sleeping.patch \
            file://0007-x86-efi-Fixup-faults-from-UEFI-firmware.patch \
            file://0008-x86-efi-Introduce-EFI_WARN_ON_ILLEGAL_ACCESSES.patch \
            file://0001-selftests-change-reboot-flag-location.patch \
            file://0001-pstore-test-remove-log-directory.patch \
            file://0001-x86-mttr-Update-only-valid-variable-range-MTRRs.patch \
           "

COMMON_CFG_x86 = " file://qemux86/modules.cfg \
                   file://qemux86/display.cfg \
                   file://qemux86/ram_block.cfg \
                   file://qemux86/debug.cfg \
                   file://qemux86/efi_x86.cfg \
                   file://qemux86/usb_hcd.cfg \
                   file://qemux86/network.cfg \
                   file://qemux86/network-devices.cfg \
                   file://qemux86/usb_ethernet.cfg \
                 "

# Add the kernel configuration fragments for x86/x86-64/arm64
SRC_URI_append_x86 = "${COMMON_CFG_x86} \
                      file://qemux86/highmem.cfg \
                     "
SRC_URI_append_x86-64 = "${COMMON_CFG_x86} \
                         file://qemux86/ndctl.cfg \
                         file://qemux86-64/linux_quirks.cfg \
                         file://qemux86-64/efi_x86-64.cfg \
                        "
SRC_URI_append_aarch64 = " file://qemuarm64/network.cfg \
                           file://qemuarm64/sbbr.cfg \
			   file://qemuarm64/64Kpages.cfg \
                         "

# pstore, highmem and efi configs are common to all the supported architectures
SRC_URI_append = "file://pstore.cfg \
                  file://pstore-test.cfg \
                  file://efi.cfg \
                  file://cpus.cfg \
                  file://lockup.cfg \
                 "

# Override KCONFIG_MODE to '--alldefconfig' from the default '--allnoconfig'
KCONFIG_MODE = 'alldefconfig'

# If a defconfig is specified via the KBUILD_DEFCONFIG variable, we copy it
# from the source tree, into a common location and normalized "defconfig" name,
# where the rest of the process will include and incoroporate it into the build

# If the fetcher has already placed a defconfig in WORKDIR (from the SRC_URI),
# we don't overwrite it, but instead warn the user that SRC_URI defconfigs take
# precendence.
KBUILD_DEFCONFIG = "defconfig"
KBUILD_DEFCONFIG_x86 = "i386_defconfig"
KBUILD_DEFCONFIG_x86-64 = "x86_64_defconfig"
LINUX_VERSION = "4.18"
LINUX_VERSION_EXTENSION = "-luv"

# Override SRCREV to point to a different commit in a bbappend file to
# build a different release of the Linux kernel.
# tag: v4.18 94710cac0ef4ee177a63b5227664b38c95bbf703
SRCREV = "94710cac0ef4ee177a63b5227664b38c95bbf703"

PR = "r5"
PV = "${LINUX_VERSION}+git${SRCPV}"

# Override COMPATIBLE_MACHINE to include your machine in a bbappend
# file. Leaving it empty here ensures an early explicit build failure.
COMPATIBLE_MACHINE = "qemux86|qemux86-64|qemuarm64"

# extract-cert.c requires openssl development files
# kernel-source/scripts/extract-cert.c:21:25:
# fatal error: openssl/bio.h: No such file or directory
DEPENDS += "openssl-native"
HOST_EXTRACFLAGS += "-I${STAGING_INCDIR_NATIVE}"

do_install_append() {
    if [ "${TARGET_ARCH}" = "x86_64" ]; then
         # There are 2 copies of the NVDIMM modules which are built. This is a
         # temporary fix to make sure the correct set of modules are used.
         rm -rf ${D}/lib/modules/${LINUX_VERSION}.0${LINUX_VERSION_EXTENSION}/kernel/drivers/nvdimm/
         rm -rf ${D}/lib/modules/${LINUX_VERSION}.0${LINUX_VERSION_EXTENSION}/kernel/drivers/dax/
         rm -rf ${D}/lib/modules/${LINUX_VERSION}.0${LINUX_VERSION_EXTENSION}/kernel/drivers/acpi/
    fi
}
