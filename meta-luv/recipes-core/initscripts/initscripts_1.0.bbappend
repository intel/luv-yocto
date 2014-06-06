FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://luv-test-manager file://luv-test-parser \
            file://luv-crash-handler"

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
}
