FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

SRC_URI += "file://interfaces-luv \
            file://ifup-lo \
           "

do_install_append () {
        install -m 0644 ${WORKDIR}/interfaces-luv ${D}${sysconfdir}/network/interfaces
        install -m 0755 ${WORKDIR}/ifup-lo ${D}${sysconfdir}/network/if-up.d
}

RDEPENDS_${PN} +="ifupdown debianutils-run-parts"
