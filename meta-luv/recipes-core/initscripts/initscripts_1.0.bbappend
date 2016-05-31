FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://luv-test-manager file://luv-test-parser \
            file://luv-crash-handler \
            file://luv-netconsole \
            file://luv-netconsole-params \
            file://luv-scripts \
            file://luv-css-styles"

RDEPENDS_${PN}+= "kernel-modules iputils iproute2 bash init-ifupdown dhcp-client"

do_install_append() {
	install -m 755 ${WORKDIR}/luv-test-manager ${D}${sysconfdir}/init.d/

	# Create runlevel link
	update-rc.d -r ${D} luv-test-manager start 99 5 .

	install -d ${D}${sysconfdir}/luv/tests
	install -d ${D}${sysconfdir}/luv/parsers

	install -m 755 ${WORKDIR}/luv-test-parser \
		${D}${sysconfdir}/luv/parsers/test-manager

	install -m 755 ${WORKDIR}/luv-crash-handler ${D}${sysconfdir}/init.d/

	# Create runlevel link for the crash handler
	update-rc.d -r ${D} luv-crash-handler start 98 3 5 .

        install -m 755 ${WORKDIR}/luv-netconsole ${D}${sysconfdir}/init.d/

        # Create runlevel link for the luv-netconsole
        update-rc.d -r ${D} luv-netconsole start 98 2 3 4 5 .

        # Install luv-netconsole-params in bin directory
        install -d ${D}${bindir}
        install -m 0755 ${WORKDIR}/luv-netconsole-params ${D}${bindir}

        # Install HTML base code files
        echo "data dir is ${datadir}"
        install -d ${D}${datadir}/luv/html
        install -m 0644 ${WORKDIR}/luv-scripts ${D}${datadir}/luv/html
        install -m 0644 ${WORKDIR}/luv-css-styles ${D}${datadir}/luv/html
}

FILES_${PN} += "${datadir}/luv/html/luv-scripts ${datadir}/luv/html/luv-css-styles"
