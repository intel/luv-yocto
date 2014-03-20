# Copyright (C) 2014 Intel Corporation
#
# This file will automatically generate and install a test runner
# script for a package.

LUV_TEST_ARGS = ""
LUV_TEST = "${PN}"

# The installation directory of test runner scripts
RUNNER_PATH = "${sysconfdir}/luv-tests"

FILES_${PN}-luv = "${RUNNER_PATH}/${PN}"

do_install_runner() {
    install -d ${D}${RUNNER_PATH}
cat > ${D}${RUNNER_PATH}/${PN} <<EOF
#!/bin/sh
#
# This is an automatically generated test runner script

${LUV_TEST} ${LUV_TEST_ARGS}
EOF
    chmod +x ${D}${RUNNER_PATH}/${PN}
}

addtask install_runner after do_install before do_package
