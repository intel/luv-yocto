DESCRIPTION = "OVMF - UEFI firmware for Qemu and KVM"
HOMEPAGE = "http://sourceforge.net/apps/mediawiki/tianocore/index.php?title=OVMF"
LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://OvmfPkg/License.txt;md5=343dc88e82ff33d042074f62050c3496"

SRC_URI = "git://github.com/tianocore/edk2.git;branch=master \
	file://0001-BaseTools-Force-tools-variables-to-host-toolchain.patch"

SRCREV="166a6552a829ab680862331449f11970c085f26d"

S = "${WORKDIR}/git"

DEPENDS_class-native="util-linux-native iasl-native"

# OVMF has trouble building with the default optimization of -O2.
BUILD_OPTIMIZATION="-pipe"

# OVMF supports IA only, although it could conceivably support ARM someday.
COMPATIBLE_HOST='(i.86|x86_64).*'

do_patch_append() {
    bb.build.exec_func('do_fix_iasl', d)
    bb.build.exec_func('do_fix_toolchain', d)
}

do_fix_iasl() {
    sed -i -e 's#/usr/bin/iasl#${STAGING_BINDIR_NATIVE}/iasl#' ${S}/BaseTools/Conf/tools_def.template
}

do_fix_toolchain(){
    sed -i -e 's#DEF(ELFGCC_BIN)/#${TARGET_PREFIX}#' ${S}/BaseTools/Conf/tools_def.template
    sed -i -e 's#DEF(GCC.*PREFIX)#${TARGET_PREFIX}#' ${S}/BaseTools/Conf/tools_def.template
    sed -i -e "s#^LINKER\(.*\)#LINKER\1\nLFLAGS += ${BUILD_LDFLAGS}#" ${S}/BaseTools/Source/C/Makefiles/app.makefile
    sed -i -e "s#^LINKER\(.*\)#LINKER\1\nCFLAGS += ${BUILD_CFLAGS}#" ${S}/BaseTools/Source/C/Makefiles/app.makefile
    sed -i -e "s#^LINKER\(.*\)#LINKER\1\nLFLAGS += ${BUILD_LDFLAGS}#" ${S}/BaseTools/Source/C/VfrCompile/GNUmakefile
    sed -i -e "s#^LINKER\(.*\)#LINKER\1\nCFLAGS += ${BUILD_CFLAGS}#" ${S}/BaseTools/Source/C/VfrCompile/GNUmakefile
}

GCC_VER="$(${CC} -v 2>&1 | tail -n1 | awk '{print $3}' | awk -F. '{print $1$2}')"

do_compile() {
    export LFLAGS="${LDFLAGS}"
    OVMF_ARCH="X64"
    if [ "${TARGET_ARCH}" != "x86_64" ] ; then
        OVMF_ARCH="IA32"
    fi
    ${S}/OvmfPkg/build.sh -a $OVMF_ARCH -b RELEASE -t GCC${GCC_VER}
}

do_install() {
    OVMF_DIR_SUFFIX="X64"
    if [ "${TARGET_ARCH}" != "x86_64" ] ; then
        OVMF_DIR_SUFFIX="Ia32" # Note the different capitalization
    fi
    install -d ${D}${datadir}/ovmf
    build_dir="${S}/Build/Ovmf$OVMF_DIR_SUFFIX/RELEASE_GCC${GCC_VER}"
    install -m 0755 ${build_dir}/FV/OVMF.fd \
	${D}${datadir}/ovmf/bios.bin
}

BBCLASSEXTEND = "native"
