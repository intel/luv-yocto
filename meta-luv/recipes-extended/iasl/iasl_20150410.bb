DESCRIPTION = "This is a cross development C compiler, assembler and linker environment for the production of 8086 executables (Optionally MSDOS COM)"
HOMEPAGE = "http://www.acpica.org/"
LICENSE = "Intel-ACPI"
LIC_FILES_CHKSUM = "file://Makefile;endline=22;md5=b15414d545d190713f1bab9023dba3be"
SECTION = "console/tools"
PR="r1"

DEPENDS="flex-native bison-native"

SRC_URI="https://acpica.org/sites/acpica/files/acpica-unix-${PV}.tar.gz"

SRC_URI[md5sum] = "7b49c79728dde65ab1ba4edbee6f0b22"
SRC_URI[sha256sum] = "1dce8d9edeb234fd553806987471f6206f429c2aab45556f62a5b2bfe2464875"

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
