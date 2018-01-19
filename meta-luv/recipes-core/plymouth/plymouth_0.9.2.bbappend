FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

RDEPENDS_${PN} = "ttf-dejavu-common ttf-dejavu-sans ttf-dejavu-sans-mono"

SRC_URI+= "file://0001-plymouth-Add-the-retain-splash-option.patch \
	   file://0001-plymouth-Change-the-plymouth-defaults.patch \
	   file://0001-plymouth-modify-the-script-theme.patch \
	   ${SPLASH_IMAGES}"

SPLASH_IMAGES = "file://luv-splash.png;outsuffix=default"

def get_target_arch(d):
 import re
 target = d.getVar('TARGET_ARCH', True)
 if target == "x86_64":
    return '7'
 elif re.match('i.86', target):
    return '6'
 elif re.match('aarch64', target):
    return '4'
 else:
    raise bb.parse.SkipPackage("TARGET_ARCH %s not supported!" % target)

num_test_suites = "${@get_target_arch(d)}"

patch() {
       sed -i s/LUV_NUM_TEST_SUITES/${num_test_suites}/ ${S}/themes/script/script.script
}

do_patch_append() {
    bb.build.exec_func('patch', d)
}

do_install_append() {
	install -d ${D}${datadir}/plymouth
	install -m 755 ${WORKDIR}/luv-splash.png ${D}/${datadir}/plymouth/
}

LOGO = "${datadir}/plymouth/luv-splash.png"
RDEPENDS_${PN}-initrd = "bash"
