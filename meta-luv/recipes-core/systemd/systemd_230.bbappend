FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

PACKAGECONFIG_append = "networkd"

SRC_URI += "file://50-oe-core.network \
            file://networking.service \
           "

do_install_append() {
        install -d ${D}${sysconfdir}/systemd/network/
        install -m 0644 ${WORKDIR}/50-oe-core.network ${D}${sysconfdir}/systemd/network/

        install -m 644 ${WORKDIR}/networking.service ${D}/${systemd_unitdir}/system
        install -d ${D}/etc/systemd/system/multi-user.target.wants
        install -m 644 ${WORKDIR}/networking.service ${D}/etc/systemd/system/multi-user.target.wants/
}

RDEPENDS_${PN} +="ifupdown debianutils"

FILES_${PN} += "${sysconfdir}/systemd/network/50-oe-core.network \
                ${systemd_unitdir}/system/networking.service \
               "
