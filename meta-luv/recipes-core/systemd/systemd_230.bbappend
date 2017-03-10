FILESEXTRAPATHS_prepend := "${THISDIR}/files:"

PACKAGECONFIG_append = "networkd"

SRC_URI += "file://50-oe-core.network \
            file://networking.service \
            file://splash_runner \
            file://splash_tty.service \
            file://0001-tty1-only-when-splash-enabled.patch \
           "

do_install_append() {
        install -d ${D}${sysconfdir}/systemd/network/
        install -m 0644 ${WORKDIR}/50-oe-core.network ${D}${sysconfdir}/systemd/network/

        install -m 644 ${WORKDIR}/networking.service ${D}/${systemd_unitdir}/system
        install -d ${D}/etc/systemd/system/multi-user.target.wants
        install -m 644 ${WORKDIR}/networking.service ${D}/etc/systemd/system/multi-user.target.wants/

        install -d ${D}${sbindir}/
        install -m 0755 ${WORKDIR}/splash_runner ${D}${sbindir}/splash_runner

        install -m 0644 ${WORKDIR}/splash_tty.service ${D}${systemd_unitdir}/system
        sed -i -e 's,@SBINDIR@,${sbindir},g' ${D}${systemd_unitdir}/system/splash_tty.service
        install -m 644 ${WORKDIR}/splash_tty.service ${D}/etc/systemd/system/multi-user.target.wants/
}

RDEPENDS_${PN} +="ifupdown debianutils"

FILES_${PN} += "${sysconfdir}/systemd/network/50-oe-core.network \
                ${systemd_unitdir}/system/networking.service \
                ${sbindir}/splash_runner \
                ${systemd_unitdir}/system/splash_tty.service \
               "
