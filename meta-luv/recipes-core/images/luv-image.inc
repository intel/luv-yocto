LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

DEPENDS_append_x86 = " bits"
DEPENDS_append_x86-64 = " bits"

HDDDIR = "${S}/hddimg"

INITRD_IMAGE_LIVE = "core-image-efi-initramfs"
IMGDEPLOYDIR = "${DEPLOY_DIR_IMAGE}"

IMAGE_FSTYPES+="hddimg"

# Tell plymouth to ignore serial consoles and limit the amount of systemD logs.
CMDLINE_USERSPACE = "systemd.log_target=null plymouth.ignore-serial-consoles"

# Kernel commandline for luv live image boot
CMDLINE_BASE = "${CMDLINE_USERSPACE} debug crashkernel=512M,high log_buf_len=1M efi=debug"

COMMON_CMDLINE_x86 = " console=ttyS0,115200 console=ttyPCH0,115200"

# A splash screen is never seen on ARM. Hence, having the splash parameter only for x86
# Nomodeset will not allow kernel to load video drivers, helps retaining splash screen.
# Make sure kdump runs before kexec after a crash and not vice-versa
COMMON_CMDLINE_x86 += "splash nomodeset crash_kexec_post_notifiers"

# Unlike the += operand, _append's do not insert a space between the current value
# and the appended string. Thus, we add them.
CMDLINE_append_aarch64 = " acpi=on"
CMDLINE_append_x86 = "${COMMON_CMDLINE_x86}"
CMDLINE_append_x86-64 = "${COMMON_CMDLINE_x86}"

LUVCFG_netconsole = "LUV_NETCONSOLE=none"
LUVCFG_storage_url = "LUV_STORAGE_URL=none"
LUVCFG_tests = "LUV_TESTS='efivarfs-test,fwts,kernel-efi-warnings,pstore-tests,"
LUVCFG_tests_append_x86-64 = "bits,chipsec,ndctl'"
LUVCFG_tests_append_aarch64 = "'"
LUVCFG_reboot_tests = "LUV_REBOOT_TESTS=none"
LUVCFG_bits_can_run = "LUV_BITS_CAN_RUN='true'"

# Path to splash screen image
FILESEXTRAPATHS_append := "${THISDIR}/luv-live-image:"

# For aarch64 depend on xorriso instead of mkisofs to remove
# a dependency on syslinux.
python() {
    import re
    target = d.getVar('TARGET_ARCH', True)
    if re.match('aarch64', target):
        d.setVar('EFI_USEXORRISO', '1')
}

SPLASH_IMAGE = "blue-luv.jpg"
SRC_URI = "file://blue-luv.jpg"

do_image_ext4() {
        :
}

do_image_ext4[noexec] = "1"
