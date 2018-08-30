do_install_append() {
	# replace the timeout 60 with timeout 20 in dhclient.conf using sed command
	sed -i -e 's/#timeout 60/timeout 20/g' ${D}${sysconfdir}/dhcp/dhclient.conf
}
