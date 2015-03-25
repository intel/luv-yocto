#
# Copyright (C) 2014 Intel Corporation.
#
# This is entirely specific to the Linux UEFI Validation (luv) project.
# We install a couple of boot loaders and a splash image.
#

def bootimg_depends(bb, d):
         import re
         deps = bb.data.getVar('TARGET_PREFIX', d, True)
         if re.search("(x86_64|i.86).*",deps):
                 return "${MLPREFIX}grub-efi"
         if re.search("aarch64",deps):
                 return "${MLPREFIX}grub"

RDEPENDS = "${@bootimg_depends(bb, d)}"
do_bootimg[depends] += "${RDEPENDS}:do_deploy"

EFI_LOADER_IMAGE = "${@base_conditional('TARGET_ARCH', 'x86_64', 'bootx64.efi', 'bootia32.efi', d)}"
EFIDIR = "/EFI/BOOT"

GRUBCFG = "${S}/grub.cfg"

efi_populate() {
    # DEST must be the root of the image so that EFIDIR is not
    # nested under a top level directory.
    DEST=$1

    install -d ${DEST}${EFIDIR}

    # Create bits directory only for x86_64 target.
    if [ "${TARGET_ARCH}" = "x86_64" ]; then
       install -d ${DEST}${EFIDIR}/bits
    fi

    # Install grub2 in EFI directory
    if [ "${TARGET_ARCH}" = "aarch64" ]; then
		install -m 0644 ${DEPLOY_DIR_IMAGE}/grubaa64.efi ${DEST}${EFIDIR}
                echo "grubaa64.efi" > ${DEST}${EFIDIR}/startup.nsh
    else
		install -m 0644 ${DEPLOY_DIR_IMAGE}/${EFI_LOADER_IMAGE} ${DEST}${EFIDIR}
    fi

    # Install BITS only for x86_64 architecture
    if [ "${TARGET_ARCH}" = "x86_64" ]; then
	    cp -r ${DEPLOY_DIR_IMAGE}/bits/boot ${DEST}
	    install -m 0644 ${DEPLOY_DIR_IMAGE}/bits/efi/boot/${EFI_LOADER_IMAGE} \
	        ${DEST}${EFIDIR}/bits/
    fi

    # Install splash and grub.cfg files into EFI directory.
    install -m 0644 ${GRUBCFG} ${DEST}${EFIDIR}

    install -m 0644 ${WORKDIR}/${SPLASH_IMAGE} ${DEST}${EFIDIR}
}

efi_iso_populate() {
    iso_dir=$1
    efi_populate $iso_dir
    # Build a EFI directory to create efi.img
    mkdir -p ${EFIIMGDIR}/${EFIDIR}
    cp -r $iso_dir/${EFIDIR}/* ${EFIIMGDIR}${EFIDIR}

    if [ "${TARGET_ARCH}" = "aarch64" ] ; then
        echo "grubaa64.efi" > ${EFIIMGDIR}/startup.nsh
        cp $iso_dir/Image ${EFIIMGDIR}
    fi
    if [ "${TARGET_ARCH}" = "x86_64" ] ; then
        echo "${GRUB_IMAGE}" > ${EFIIMGDIR}/startup.nsh
        cp $iso_dir/vmlinuz ${EFIIMGDIR}
    fi

    if [ -f "$iso_dir/initrd" ] ; then
        cp $iso_dir/initrd ${EFIIMGDIR}
    fi
}

efi_hddimg_populate() {
    efi_populate $1
}

python build_efi_cfg() {
    path = d.getVar('GRUBCFG', True)
    if not path:
        raise bb.build.FuncFailed('Unable to read GRUBCFG')

    try:
        cfgfile = file(path, 'w')
    except OSError:
        raise bb.build.funcFailed('Unable to open %s' % (cfgfile))

    if "${TARGET_ARCH}" == "x86_64":
       cfgfile.write('default=bits\n')
       cfgfile.write('timeout=0\n')
       cfgfile.write('fallback=0\n')

    cfgfile.write('menuentry \'luv\' {\n')
    if "${TARGET_ARCH}" == "x86_64":
       cfgfile.write('linux /vmlinuz')
    if "${TARGET_ARCH}" == "aarch64":
        cfgfile.write('linux /Image')

    append = d.getVar('APPEND', True)
    if append:
        cfgfile.write('%s' % (append))

    cfgfile.write('\n')

    cfgfile.write('initrd /initrd\n')
    cfgfile.write('}\n')

    loader = d.getVar('EFI_LOADER_IMAGE', True)
    if not loader:
        raise bb.build.FuncFailed('Unable to find EFI_LOADER_IMAGE')

    if "${TARGET_ARCH}" == "x86_64":
       cfgfile.write('menuentry \'bits\' {\n')
       cfgfile.write('chainloader /EFI/BOOT/bits/%s\n' % loader)
       cfgfile.write('}\n')

    cfgfile.close()
}
