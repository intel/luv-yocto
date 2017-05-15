#
# Copyright (C) 2014 Intel Corporation.
#
# This is entirely specific to the Linux UEFI Validation (luv) project.
# We install a couple of boot loaders and a splash image.
# Also, we sign the grub2 so that it can be launched by shim
#

# The x86 luv-netboot-image depend on bits package.  But the AArch64 is not.
def get_bits_depends(bb, d):
         import re
         deps = bb.data.getVar('TARGET_PREFIX', d, True)
         if re.search("(x86_64|i.86).*",deps):
                 return "bits:do_deploy"
         if re.search("aarch64",deps):
                 return ""
_BITSDEPENDS = "${@get_bits_depends(bb, d)}"

# The x86 build is depends on grub-efi and AArch64 build depends on grub_git
def get_grub_depends(bb, d):
         import re
         deps = bb.data.getVar('TARGET_PREFIX', d, True)
         if re.search("(x86_64|i.86).*",deps):
                 return "${MLPREFIX}grub-efi"
         if re.search("aarch64",deps):
                 return "${MLPREFIX}grub"

_RDEPENDS = "${@get_grub_depends(bb, d)}"
do_bootimg[depends] += "${_RDEPENDS}:do_deploy \
                        sbsigntool-native:do_populate_sysroot"

EFI_LOADER_IMAGE = "${@base_conditional('TARGET_ARCH', 'x86_64', 'bootx64.efi', 'bootia32.efi', d)}"
EFIDIR = "/EFI/BOOT"

LUV_FOR_NETBOOT="${@bb.utils.contains('DISTRO_FEATURES', 'luv-netboot','1' , '0', d)}"

GRUBCFG = "${S}/grub.cfg"
LUV_CFG = "${S}/luv.cfg"

def extra_initrd(d):
    import re

    target = d.getVar('TARGET_ARCH', True)
    if re.search('86', target):
        return '/boot/bitsrd'
    else:
        return ''

EXTRA_INITRD = "${@extra_initrd(d)}"

efi_populate() {
    # DEST must be the root of the image so that EFIDIR is not
    # nested under a top level directory.
    DEST=$1

    install -d ${DEST}${EFIDIR}

    # Install grub2 in EFI directory
    if [ "${TARGET_ARCH}" = "aarch64" ]; then
		install -m 0644 ${DEPLOY_DIR_IMAGE}/bootaa64.efi ${DEST}${EFIDIR}
                echo "bootaa64.efi" > ${DEST}${EFIDIR}/startup.nsh

    # TODO: need conditional signing; e.g., if (DISTRO_FEATURES contains secure_boot)
    # shim bootloader does not seem to work with i386. Thus we don't use it for 32-bit
    elif [ "${TARGET_ARCH}" = "x86_64" ] && [ "${LUV_FOR_NETBOOT}" = "0"  ]; then
                # sign grub2 bootloader
                sbsign --key ${DEPLOY_DIR_IMAGE}/LUV.key --cert ${DEPLOY_DIR_IMAGE}/LUV.crt \
                       --output ${DEPLOY_DIR_IMAGE}/grubx64.efi ${DEPLOY_DIR_IMAGE}/${EFI_LOADER_IMAGE}

                # temporarily rename the unsigned grub2 bootloader
                mv ${DEPLOY_DIR_IMAGE}/${EFI_LOADER_IMAGE} ${DEPLOY_DIR_IMAGE}/${EFI_LOADER_IMAGE}-unsigned
                # shim will become our main bootloader
                mv ${DEPLOY_DIR_IMAGE}/shim.efi  ${DEPLOY_DIR_IMAGE}/${EFI_LOADER_IMAGE}

                # install everything
                install -m 0644 ${DEPLOY_DIR_IMAGE}/${EFI_LOADER_IMAGE} ${DEST}${EFIDIR}
                install -m 0644 ${DEPLOY_DIR_IMAGE}/grubx64.efi ${DEST}${EFIDIR}
                install -m 0644 ${DEPLOY_DIR_IMAGE}/MokManager.efi ${DEST}${EFIDIR}
                install -m 0644 ${DEPLOY_DIR_IMAGE}/LUV.cer ${DEST}

                # restore files to leave all in good shape for all the callers of the funciton
                mv ${DEPLOY_DIR_IMAGE}/${EFI_LOADER_IMAGE} ${DEPLOY_DIR_IMAGE}/shim.efi
                mv ${DEPLOY_DIR_IMAGE}/${EFI_LOADER_IMAGE}-unsigned ${DEPLOY_DIR_IMAGE}/${EFI_LOADER_IMAGE}
    else
		install -m 0644 ${DEPLOY_DIR_IMAGE}/${EFI_LOADER_IMAGE} ${DEST}${EFIDIR}
    fi

    if echo "${TARGET_ARCH}" | grep -q "i.86" || [ "${TARGET_ARCH}" = "x86_64" ]; then
        efi_populate_bits ${DEST}
    fi

    # Install splash and grub.cfg files into EFI directory.
    install -m 0644 ${GRUBCFG} ${DEST}${EFIDIR}

    install -m 0644 ${WORKDIR}/${SPLASH_IMAGE} ${DEST}${EFIDIR}

    # Install luv.cfg file in to the boot parition.
    install -m 0644 ${LUV_CFG} ${DEST}
}

efi_populate_bits() {
    DEST=$1
    # TODO: weird behavior here. When building luv-live-image,
    #   cp -r -v ${DEPLOY_DIR_IMAGE}/bits/boot ${DEST}
    # copies the boot directory into ${DEST} without issue. However,
    # the same line when building for luv-netboot-image copies the _contents_
    # of the the boot directory into ${DEST}. For now, perform the copy
    # manually.
    install -d ${DEST}/boot
    cp -r ${DEPLOY_DIR_IMAGE}/bits/boot/* ${DEST}/boot

    # install EFI GRUB image, sign if needed
    # TODO: Need condiitional signing based on DISTRO_FEATURES
    install -d ${DEST}${EFIDIR}/bits

    if [ "${LUV_FOR_NETBOOT}" = "0" ]; then
        mv ${DEPLOY_DIR_IMAGE}/bits/efi/boot/${EFI_LOADER_IMAGE} \
           ${DEPLOY_DIR_IMAGE}/bits/efi/boot/${EFI_LOADER_IMAGE}-unsigned

        sbsign --key ${DEPLOY_DIR_IMAGE}/LUV.key --cert ${DEPLOY_DIR_IMAGE}/LUV.crt \
               --output ${DEPLOY_DIR_IMAGE}/bits/efi/boot/${EFI_LOADER_IMAGE} \
               ${DEPLOY_DIR_IMAGE}/bits/efi/boot/${EFI_LOADER_IMAGE}-unsigned

        install -m 0644 ${DEPLOY_DIR_IMAGE}/bits/efi/boot/${EFI_LOADER_IMAGE} \
                ${DEST}${EFIDIR}/bits/
        # restore files
        rm ${DEPLOY_DIR_IMAGE}/bits/efi/boot/${EFI_LOADER_IMAGE}
        mv ${DEPLOY_DIR_IMAGE}/bits/efi/boot/${EFI_LOADER_IMAGE}-unsigned \
           ${DEPLOY_DIR_IMAGE}/bits/efi/boot/${EFI_LOADER_IMAGE}
    else
        install -m 0644 ${DEPLOY_DIR_IMAGE}/bits/efi/boot/${EFI_LOADER_IMAGE} \
                ${DEST}${EFIDIR}/bits/
    fi
}

efi_iso_populate() {
    iso_dir=$1
    efi_populate $iso_dir
    # Build a EFI directory to create efi.img
    mkdir -p ${EFIIMGDIR}/${EFIDIR}
    cp -r $iso_dir/${EFIDIR}/* ${EFIIMGDIR}${EFIDIR}

    if [ "${TARGET_ARCH}" = "aarch64" ] ; then
        echo "bootaa64.efi" > ${EFIIMGDIR}/startup.nsh
    fi
    if echo "${TARGET_ARCH}" | grep -q "i.86" || [ "${TARGET_ARCH}" = "x86_64" ]; then
        echo "${GRUB_IMAGE}" > ${EFIIMGDIR}/startup.nsh
    fi

    cp $iso_dir/vmlinuz ${EFIIMGDIR}
    if [ -f "$iso_dir/initrd" ] ; then
        cp $iso_dir/initrd ${EFIIMGDIR}
    fi
}

efi_hddimg_populate() {
    efi_populate $1
}

python build_efi_cfg() {
    import re

    path = d.getVar('GRUBCFG', True)
    if not path:
        raise bb.build.FuncFailed('Unable to read GRUBCFG')

    try:
        cfgfile = open(path, 'w')
    except OSError:
        raise bb.build.funcFailed('Unable to open %s' % (cfgfile))

    target = d.getVar('TARGET_ARCH', True)
    cfgfile.write('source /luv.cfg\n')
    cfgfile.write('timeout=0\n')

    if re.search("(x86_64|i.86)", target):
       cfgfile.write('default=bits\n')
       cfgfile.write('fallback=0\n')

    cfgfile.write('menuentry \'luv\' {\n')
    cfgfile.write('linux /vmlinuz ')

    cmdline = d.getVar('CMDLINE', True)
    if cmdline:
        cfgfile.write('%s' % (cmdline))

    cfgfile.write(' luv_netconsole=${LUV_NETCONSOLE}')
    cfgfile.write(' luv_storage=${LUV_STORAGE_URL}')

    cfgfile.write('\n')

    cfgfile.write('initrd /initrd')
    extra_initrd = d.getVar('EXTRA_INITRD', True)
    cfgfile.write(' %s' % extra_initrd)
    cfgfile.write('\n}\n')

    loader = d.getVar('EFI_LOADER_IMAGE', True)
    if not loader:
        raise bb.build.FuncFailed('Unable to find EFI_LOADER_IMAGE')

    if re.search("(x86_64|i.86)", target):
       cfgfile.write('menuentry \'bits\' {\n')
       cfgfile.write('chainloader /EFI/BOOT/bits/%s\n' % loader)
       cfgfile.write('}\n')

    cfgfile.close()
}

# define a function that takes name and comment as arguments
def insert_var(name, comment, d):
    name = d.getVar(name, True)
    if name:
        value = ('%s' %(name))
    else:
        value = ' '
    var = '# ' + comment + '\n'
    var = var + 'set ' + value + '\n'
    return var

python build_luv_cfg() {
    import re

    path = d.getVar('LUV_CFG', True)
    if not path:
        raise bb.build.FuncFailed('Unable to read LUV_CFG variable')

    try:
        luvcfg = open(path, 'w')
    except OSError:
        raise bb.build.funcFailed('Unable to open %s' % (luvcfg))

    # Beginning of the file luv.cfg
    luvcfg.write('## Start of luv.cfg ##\n\n')

    comment = 'This is the parameter to listen to the client that boots LUV'
    name = 'LUVCFG_netconsole'
    luvcfg.write(insert_var(name, comment, d))

    comment= "This is the parameter for url of the server/website"
    name = 'LUVCFG_storage_url'
    luvcfg.write(insert_var(name, comment, d))

    # pad with spaces only if luv-netboot is present, so to make EFI binary
    # bootable even after making changes to it
    luvcfg_luv_netboot = d.getVar('DISTRO_FEATURES', True)
    if luvcfg_luv_netboot:
        if re.search('luv-netboot', luvcfg_luv_netboot):
            x = ' '
            luvcfg.write(1000*x)

    # end of the file luv.cfg
    luvcfg.write('\n## END of luv.cfg ##')

    luvcfg.close()
}

create_symlinks() {
	cd ${DEPLOY_DIR_IMAGE}

	rm -f ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.iso
	ln -s ${IMAGE_NAME}.iso ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.iso

	rm -f ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.hddimg
	ln -s ${IMAGE_NAME}.hddimg ${DEPLOY_DIR_IMAGE}/${IMAGE_LINK_NAME}.hddimg
}
