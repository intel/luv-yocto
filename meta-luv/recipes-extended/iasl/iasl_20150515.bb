DESCRIPTION = "This is a cross development C compiler, assembler and linker environment for the production of 8086 executables (Optionally MSDOS COM)"
HOMEPAGE = "http://www.acpica.org/"
LICENSE = "Intel-ACPI"
LIC_FILES_CHKSUM = "file://Makefile;endline=22;md5=b15414d545d190713f1bab9023dba3be"
SECTION = "console/tools"
PR="r1"

DEPENDS="flex-native bison-native"

SRC_URI="https://acpica.org/sites/acpica/files/acpica-unix-${PV}.tar.gz"

SRC_URI[md5sum] = "c8c128b2d4859b52bc9c802faba2e908"
SRC_URI[sha256sum] = "bfa1f296a3cc13421331dbaad3b62e0184678cc312104c3e8ac799ead0742c45"

S="${WORKDIR}/acpica-unix-${PV}"

NATIVE_INSTALL_WORKS = "1"
BBCLASSEXTEND = "native"

do_compile() {
	make iasl
}

do_install() {
	mkdir -p ${D}${prefix}/bin
	cp ${S}/generate/unix/bin/iasl ${D}${prefix}/bin
}
