DESCRIPTIONS = "xorriso copies file objects from POSIX compliant filesystems into Rock Ridge enhanced ISO 9660 filesystems and allows session-wise manipulation of such filesystems"

LICENSE = "GPLv3"
LIC_FILES_CHKSUM = "file://COPYING;md5=d32239bcb673463ab874e80d47fae504"

# with, without, depends, rdepends
#
PACKAGECONFIG ??= ""
PACKAGECONFIG[acl] = "--enable-libacl,--disable-libacl,acl,"
PACKAGECONFIG[bzip2] = "--enable-libbz2,--disable-libbz2,bzip2,"
PACKAGECONFIG[readline] = "--enable-libreadline,--disable-libreadline,readline,"

inherit autotools-brokensep deploy

SRC_URI = "http://www.gnu.org/software/${BPN}/${BPN}-${PV}.tar.gz"

do_configure () {
    touch NEWS
    libtoolize --force
    aclocal
    autoheader
    automake --force-missing --add-missing
    autoconf
    oe_runconf
}

BBCLASSEXTEND = "native"

SRC_URI[md5sum] = "a0fc534ef8f9589282a189f0e6cdf952"
SRC_URI[sha256sum] = "901204634651533f7cbd105eab560534702458258529aac4b2f0fc946992107e"
