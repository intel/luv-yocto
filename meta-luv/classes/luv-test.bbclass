# Copyright (C) 2014 Intel Corporation
#
# This file will automatically generate and install a test runner
# script for a package.

LUV_TEST_ARGS = ""
LUV_TEST = "${PN}"

LUV_TEST_LOG_PARSER = ""

# The installation directory of test runner scripts and log parsers
RUNNER_PATH = "${sysconfdir}/luv/tests"
PARSER_PATH = "${sysconfdir}/luv/parsers"

FILES_${PN} += "${RUNNER_PATH}/${PN} ${PARSER_PATH}/${PN}"

do_install_append() {
    runner_dir="${D}${RUNNER_PATH}"
    install -d $runner_dir

    log_dir="${D}${PARSER_PATH}"
    install -d $log_dir

    if [ ! -z ${LUV_TEST_LOG_PARSER} ]; then
        parser="${PARSER_PATH}/${PN}"
        install -m 755 ${WORKDIR}/${LUV_TEST_LOG_PARSER} ${D}${parser}
    fi

    cat > ${runner_dir}/${PN} <<EOF
#!/bin/sh
#
# This is an automatically generated test runner script that is invoked
# by luv-test-manager.
#
# Anything we write to stdout must be in the standard luv-test log
# format.

${LUV_TEST} ${LUV_TEST_ARGS}
EOF
    chmod +x ${runner_dir}/${PN}
}
