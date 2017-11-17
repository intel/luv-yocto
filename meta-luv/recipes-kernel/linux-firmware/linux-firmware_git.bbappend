# linux-firmware add all the licenses for the available firmware binaries.
# We keep only the licenses for the ones we include.

LICENSE_${PN}-luv = "\
      Firmware-cavium \
    & Firmware-chelsio_firmware \
    & Firmware-e100 \
    & Firmware-kaweth \
    & Firmware-myri10ge_firmware \
    & Firmware-phanfw \
    & Firmware-atmel \
"

PACKAGES =+ "${PN}-luv"

FILES_${PN}-luv = " ${nonarch_base_libdir}/firmware/atmel/* \
		    ${nonarch_base_libdir}/firmware/phanfw.bin \
		    ${nonarch_base_libdir}/firmware/myri10ge_rss_ethp_z8e.dat \
		    ${nonarch_base_libdir}/firmware/myri10ge_rss_eth_z8e.dat \
		    ${nonarch_base_libdir}/firmware/myri10ge_ethp_z8e.dat \
		    ${nonarch_base_libdir}/firmware/myri10ge_eth_z8e.dat \
		    ${nonarch_base_libdir}/firmware/kaweth/* \
		    ${nonarch_base_libdir}/firmware/e100/* \
		    ${nonarch_base_libdir}/firmware/LICENCE.cavium \
		    ${nonarch_base_libdir}/firmware/LICENCE.chelsio_firmware \
		    ${nonarch_base_libdir}/firmware/LICENCE.e100 \
		    ${nonarch_base_libdir}/firmware/LICENCE.kaweth \
		    ${nonarch_base_libdir}/firmware/LICENCE.mwl8335 \
		    ${nonarch_base_libdir}/firmware/LICENCE.myri10ge_firmware \
		    ${nonarch_base_libdir}/firmware/LICENCE.phanfw \
		    ${nonarch_base_libdir}/firmware/LICENSE.atmel \
		  "
