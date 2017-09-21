SUMMARY = "Signed shim is a lightweight EFI application signed by Microsoft Corporation"

DESCRIPTION = "The UEFI Secure Boot feature requires that any binary executed by the \
firmware to be signed. It may be possible for the machine owner to provide his/her own \
certificate for his/her own binaries. However, most of the systems available that feature \
Secure Boot comes preloaded with certificates issued by Microsoft Corporation, the \
official organization used by the UEFI forum. As indicated by its name, signed shim \
was signed by Microsoft Corporation. Thus, it signatue will be recognized by UEFI firmware. \
Once executed, the signed shim will either look for a signed bootloader ot it will \
present the user with the option of entering a certificate for the signed binary of his/her \
own."

# TODO: The signed binary is not distributed with any license file. However, the source
# does have a copyright file that allow redistribution of binaries and source.
HOMEPAGE = "http://mjg59.dreamwidth.org/20303.html"
LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/GPL-2.0;md5=801f80980d171dd6425610833a22dbe6"

DEPENDS = "openssl-native"

inherit deploy


SRC_URI = "http://www.codon.org.uk/~mjg59/shim-signed/shim-signed-0.2.tgz"
S = "${WORKDIR}"

SRC_URI[md5sum] = "879108b45e7d1e6794673af29b27517b"
SRC_URI[sha256sum] = "4c522aa6d9d20308d35907a03c855bdaa12a301d0b9b8c86b357e3808ae3f8b3"

do_deploy() {

       cp ${WORKDIR}/shim-signed/shim.efi ${DEPLOYDIR}
       cp ${WORKDIR}/shim-signed/MokManager.efi ${DEPLOYDIR}
       # key to sign binaries
       cp ${S}/LUV.key ${DEPLOYDIR}
       # certificate used by sbsign
       cp ${S}/LUV.crt ${DEPLOYDIR}
       # the same certificate used by MokManager.efi
       cp ${S}/LUV.cer ${DEPLOYDIR}
}

do_configure() {
      openssl req -new -x509 -newkey rsa:2048 -keyout LUV.key -out LUV.crt -nodes -days 20 \
            -subj "/CN=Linux UEFI Validation/"
      openssl x509 -in LUV.crt -out LUV.cer -outform DER
}

do_compile[noexec] = "1"
do_install[noexec] = "1"
do_populate_sysroot[noexec] = "1"

addtask deploy after do_configure
