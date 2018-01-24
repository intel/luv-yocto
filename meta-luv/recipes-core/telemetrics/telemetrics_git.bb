SUMMARY = "telemetrics-client"

DESCRIPTION = "The telemetrics-client package provides the front end \
component of a complete telemetrics solution for Linux-based operating \
systems. This includes telemetrics probes that collect specific types \
of data, a daemon, telemd, that prepares the records to send to a \
telemetrics server and a library to create telemetrics records and \
send them to the daemon for further processing."

LICENSE = "GPLv2"
LIC_FILES_CHKSUM = "file://LICENSE.LGPL-2.1;md5=4fbd65380cdd255951079008b364516c"

SRC_URI = "git://github.com/clearlinux/telemetrics-client \
           file://0001-change-server-URL.patch \
           file://0001-start-telemetric-daemon-after-network.patch \
           file://0001-configure-remove-malloc.patch \
           file://telemetrics_runner \
           file://telemetrics_script.service \
           file://0001-change-version_id.patch \
           file://0001-Add-Kernel-panic-to-the-list.patch \
           file://0001-pstore-start-after-network.patch \
           file://config.site \
           file://0001-pstore-clean-Run-pstore-clean-only-when-luv.telemetr.patch \
          "

SRCREV="3d24527096bc14ca8b425f66daed4547b9ee93da"

EXTRA_OECONF = "--prefix=/usr --libdir=/usr/lib --localstatedir=/var"

DEPENDS = "glib-2.0 libcheck curl glibc elfutils systemd"

RDEPENDS_${PN} = "bash"

S = "${WORKDIR}/git"

inherit autotools-brokensep
inherit pkgconfig
inherit autotools
inherit systemd

do_configure_prepend() {
    sed -i -e 's,CFLAGS1,${CFLAGS},g' ${WORKDIR}/config.site
    sed -i -e 's,CPPFLAGS1,${CPPFLAGS},g' ${WORKDIR}/config.site
    CONFIG_SITE="${WORKDIR}/config.site"
}

SYSTEMD_PACKAGES = "${PN}"
SYSTEMD_SERVICE_${PN} = " \
    telemd.service \
    hprobe.service \
    hprobe.timer \
    oops-probe.service \
    pstore-probe.service \
    klogscanner.service \
    journal-probe.service \
    pstore-clean.service \
    telemetrics_script.service \
"

inherit useradd
USERADD_PACKAGES = "${PN}"
USERADD_PARAM_${PN} = "telemetry"
# Android kernel fix
inherit extrausers
GROUPADD_PARAM_${PN} = "-g 3003 inet"
EXTRA_USERS_PARAMS = "usermod -a -G 3003 telemetry;"

do_install_append() {
	install -d ${D}/var/spool/telemetry/
	install -d ${D}/var/cache/telemetry/
	install -d ${D}/var/cache/telemetry/pstore/
	install -d ${D}/var/cache/telemetry/oops/
	install -d ${D}${sbindir}/
	install -d ${D}${systemd_unitdir}/system
	install -m 0644 ${WORKDIR}/telemetrics_script.service ${D}${systemd_unitdir}/system
	sed -i -e 's,@SBINDIR@,${sbindir},g' ${D}${systemd_unitdir}/system/telemetrics_script.service
	install -m 755 ${WORKDIR}/telemetrics_runner ${D}${sbindir}/
}

FILES_${PN} += "/usr/share/defaults/telemetrics/* \
		/var/spool/telemetry/ \
		/var/cache/telemetry/ \
		${sbindir}/telemetrics_runner \
		${systemd_unitdir}/system/* \
		/usr/lib/libtelemetry.so.3.0.0 \
		/usr/lib/pkgconfig/libtelemetry.pc \
		/usr/lib/libtelemetry.la \
		/usr/lib/libtelemetry.so.3 \
		/usr/lib/systemd \
		/usr/lib/sysctl.d \
		/usr/lib/tmpfiles.d \
		/usr/lib/systemd/system.conf.d \
		/usr/lib/systemd/system.conf.d/40-core-ulimit.conf \
		/usr/lib/sysctl.d/40-crash-probe.conf \
		/usr/lib/tmpfiles.d/telemetrics-dirs.conf"
