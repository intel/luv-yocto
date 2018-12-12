SUMMARY = "JQ"
DESCRIPTION = "A lightweight and flexible command-line JSON processor"
HOMEPAGE = "https://stedolan.github.io/jq/"
LICENSE = "CC-BY-3.0"
LIC_FILES_CHKSUM = "file://COPYING;md5=29dd0c35d7e391bb8d515eacf7592e00"

inherit autotools-brokensep

PV = "jq-1.5"

DEPENDS = "autoconf libtool automake flex-native bison-native"

EXTRA_AUTORECONF += " -i"

#SRCREV = "a5b5cbefb83935ce95ec62b9cadc8ec73026d33a"
SRCREV = "365c1000e7094ad1ffdd60130c9d477959894086"
SRC_URI = "gitsm://github.com/stedolan/jq.git;protocol=http;branch=${PV}-branch"

S = "${WORKDIR}/git"

