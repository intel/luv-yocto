FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}:"

SRC_URI += "file://luv-test-manager"

do_install_append() {
	install -m 755 ${WORKDIR}/luv-test-manager ${D}${sysconfdir}/init.d/

	# Create runlevel link
	update-rc.d -r ${D} luv-test-manager start 99 5 .

	install -d ${D}${sysconfdir}/luv-tests/
}
