FILESEXTRAPATHS_prepend := "${THISDIR}/ndctl:"

RDEPENDS_${PN} = "nvdimm-test"

inherit luv-test

SRC_URI += "file://luv-parser-ndctl"
SRC_URI += "file://ndctl_runner"
SRC_URI += "file://ndctl.json"

do_install_append() {
        install -d ${D}${bindir}
        install -m 755 ${WORKDIR}/ndctl_runner ${D}${bindir}
}

LUV_TEST_LOG_PARSER="luv-parser-ndctl"
LUV_TEST_JSON="ndctl.json"
LUV_TEST="ndctl_runner"
LUV_TEST_ARGS=""
