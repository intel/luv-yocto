# Copyright (C) 2014 Intel Corporation
#
# This file will automatically generate and install a test runner
# script for a package.

LUV_TEST_ARGS ?= ""
LUV_TEST ?= "${PN}"

python __anonymous() {
    json = d.getVar('LUV_TEST_JSON')
    if (len(json) > 0):
        d.appendVar("RDEPENDS_%s" % d.getVar('PN'), " jq")
}

LUV_TEST_LOG_PARSER ?= ""
LUV_TEST_JSON ?= ""

python __anonymous() {
    json = d.getVar('LUV_TEST_JSON')
    if (len(json) > 0):
        d.appendVar("RDEPENDS_%s" % d.getVar('PN'), " jq")
}

# The installation directory of test runner scripts and log parsers
runnerdir = "${sysconfdir}/luv/tests"
parserdir = "${sysconfdir}/luv/parsers"

FILES_${PN} += "${RUNNER_PATH}/${PN} ${PARSER_PATH}/${PN}"

do_install_append() {
    install -d ${D}${runnerdir}
    install -d ${D}${parserdir}

    if [ ! -z ${LUV_TEST_LOG_PARSER} ]; then
        install -m 755 ${WORKDIR}/${LUV_TEST_LOG_PARSER} ${D}${parserdir}/${PN}
        sed -i -e 's@PARSERDIR@${parserdir}@g' ${D}${parserdir}/${PN}
    fi

    if [ ! -z ${LUV_TEST_JSON} ]; then
        install -m 644 ${WORKDIR}/${LUV_TEST_JSON} ${D}${parserdir}
    fi

    cat > ${D}${runnerdir}/${PN} <<EOF
#!/bin/sh
#
# This is an automatically generated test runner script that is invoked
# by luv-test-manager.
#
# Anything we write to stdout must be in the standard luv-test log
# format.

${LUV_TEST} ${LUV_TEST_ARGS}
EOF
    chmod +x ${D}${runnerdir}/${PN}
}
