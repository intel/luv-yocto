SUMMARY = "LUV scripts"

DESCRIPTION = " This recipe installs the scripts which LUV will run during \
boot up, i.e the netconsole, the crash handler and the test manager along with \
some helper scripts (parser, css styling). Also, it installs the corresponding \
systemd unit files to establish the order in which these scripts are run. "

HOMEPAGE = "https://github.com/01org/luv-yocto/"

BUGTRACKER = "https://github.com/01org/luv-yocto/issues"

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SYSTEMD_PACKAGES =+ "${PN}-manager ${PN}-netconsole ${PN}-crash"
PACKAGES =+ "${PN}-manager ${PN}-netconsole ${PN}-crash"

SYSTEMD_SERVICE_${PN}-manager = "luv-test-manager.service"
SYSTEMD_SERVICE_${PN}-netconsole = "luv-netconsole.service"
SYSTEMD_SERVICE_${PN}-crash = "luv-crash-handler.service"

inherit systemd
FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://luv-test-manager file://luv-test-parser \
            file://luv-crash-handler \
            file://luv-netconsole \
            file://luv-netconsole-params \
            file://luv-scripts \
            file://luv-css-styles \
            file://submit_results \
            file://luv-test-manager.service \
            file://luv-crash-handler.service \
            file://luv-netconsole.service \
            file://luv-dmesg-acpi-tables-dump \
          "

RDEPENDS_${PN}-netconsole+= "kernel-modules curl iputils iproute2 bash init-ifupdown dhcp-client"
RDEPENDS_${PN}-crash+="bash"
RDEPENDS_${PN}-manager+= "bash"
RDEPENDS_${PN}+= "bash gzip"

do_install_append() {
       install -d ${D}${sbindir}/
       install -d ${D}${sysconfdir}/init.d/
       install -m 755 ${WORKDIR}/luv-test-manager ${D}${sbindir}/
       install -d ${D}${sysconfdir}/luv/tests
       install -d ${D}${sysconfdir}/luv/parsers

       install -m 755 ${WORKDIR}/luv-test-parser \
               ${D}${sysconfdir}/luv/parsers/test-manager

       install -m 755 ${WORKDIR}/luv-crash-handler ${D}${sbindir}/

       install -m 755 ${WORKDIR}/luv-netconsole ${D}${sbindir}/

       # Install luv-netconsole-params in bin directory
       install -d ${D}${bindir}
       install -m 0755 ${WORKDIR}/luv-netconsole-params ${D}${bindir}

       # Install submit_results in bin directory
       install -d ${D}${bindir}
       install -m 0755 ${WORKDIR}/submit_results ${D}${bindir}

       # Install HTML base code files
       echo "data dir is ${datadir}"
       install -d ${D}${datadir}/luv/html
       install -m 0644 ${WORKDIR}/luv-scripts ${D}${datadir}/luv/html
       install -m 0644 ${WORKDIR}/luv-css-styles ${D}${datadir}/luv/html

       install -d ${D}${systemd_unitdir}/system

       install -m 0644 ${WORKDIR}/luv-test-manager.service ${D}${systemd_unitdir}/system
       sed -i -e 's,@SBINDIR@,${sbindir},g' ${D}${systemd_unitdir}/system/luv-test-manager.service

       install -m 0644 ${WORKDIR}/luv-netconsole.service ${D}${systemd_unitdir}/system
       sed -i -e 's,@SBINDIR@,${sbindir},g' ${D}${systemd_unitdir}/system/luv-netconsole.service

       install -m 0644 ${WORKDIR}/luv-crash-handler.service ${D}${systemd_unitdir}/system
       sed -i -e 's,@SBINDIR@,${sbindir},g' ${D}${systemd_unitdir}/system/luv-crash-handler.service

       # Install luv-dmesg-acpi-tables-dump in bin directory
       install -d ${D}${bindir}
       install -m 0755 ${WORKDIR}/luv-dmesg-acpi-tables-dump ${D}${bindir}
}

FILES_${PN}-manager += "${datadir}/luv/html/luv-scripts \
			${datadir}/luv/html/luv-css-styles \
			${systemd_unitdir}/system/luv-test-manager.service \
			${sbindir}/luv-test-manager"
FILES_${PN}-crash += "${systemd_unitdir}/system/luv-crash-handler.service \
		      ${sbindir}/luv-crash-handler \
		      ${sysconfdir}/luv/parsers/test-manager"
FILES_${PN}-netconsole += "${systemd_unitdir}/system/luv-netconsole.service \
			   ${sbindir}/luv-netconsole ${bindir}/luv-netconsole-params"
FILES_${PN} += "${sysconfdir}/luv/parsers/test-manager \
		${bindir}/submit_results"
