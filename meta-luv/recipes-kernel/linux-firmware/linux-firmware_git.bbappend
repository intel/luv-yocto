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

FILES_${PN}-luv = " ${nonarch_base_libdir}/firmware/atmel/wilc1000_ap_fw.bin \
                    ${nonarch_base_libdir}/firmware/atmel/wilc1000_fw.bin \
                    ${nonarch_base_libdir}/firmware/atmel/wilc1000_p2p_fw.bin \
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
                    ${nonarch_base_libdir}/firmware/cis/* \
                    ${nonarch_base_libdir}/firmware/3com/typhoon.bin \
                    ${nonarch_base_libdir}/firmware/acenic/* \
                    ${nonarch_base_libdir}/firmware/adaptec/* \
                    ${nonarch_base_libdir}/firmware/bnx2/bnx2-mips-06-6.2.3.fw \
                    ${nonarch_base_libdir}/firmware/bnx2x/bnx2x-e1-7.12.30.0.fw \
                    ${nonarch_base_libdir}/firmware/bnx2x/bnx2x-e1h-7.12.30.0.fw \
                    ${nonarch_base_libdir}/firmware/bnx2x/bnx2x-e2-7.12.30.0.fw \
                    ${nonarch_base_libdir}/firmware/cbfw-3.2.3.0.bin \
                    ${nonarch_base_libdir}/firmware/cis/* \
                    ${nonarch_base_libdir}/firmware/ct2fw-3.2.5.1.bin \
                    ${nonarch_base_libdir}/firmware/ctfw-3.2.5.1.bin \
                    ${nonarch_base_libdir}/firmware/cxgb3/ael20* \
                    ${nonarch_base_libdir}/firmware/cxgb3/t3b_psram-1.1.0.bin \
                    ${nonarch_base_libdir}/firmware/cxgb3/t3c_psram-1.1.0.bin \
                    ${nonarch_base_libdir}/firmware/cxgb3/t3fw-7.12.0.bin \
                    ${nonarch_base_libdir}/firmware/cxgb4/* \
                    ${nonarch_base_libdir}/firmware/GPL-3 \
                    ${nonarch_base_libdir}/firmware/LICENCE.Netronome \
                    ${nonarch_base_libdir}/firmware/LICENCE.rockchip \
                    ${nonarch_base_libdir}/firmware/liquidio/* \
                    ${nonarch_base_libdir}/firmware/mt7662.bin \
                    ${nonarch_base_libdir}/firmware/mt7662_rom_patch.bin \
                    ${nonarch_base_libdir}/firmware/myricom/lanai.bin \
                    ${nonarch_base_libdir}/firmware/ositech/Xilinx7OD.bin \
                    ${nonarch_base_libdir}/firmware/phanfw.bin \
                    ${nonarch_base_libdir}/firmware/qed/* \
                    ${nonarch_base_libdir}/firmware/README \
                    ${nonarch_base_libdir}/firmware/rockchip/dptx.bin \
                    ${nonarch_base_libdir}/firmware/rtl_nic/* \
                    ${nonarch_base_libdir}/firmware/slicoss/* \
                    ${nonarch_base_libdir}/firmware/sun/cassini.bin \
                    ${nonarch_base_libdir}/firmware/tehuti/bdx.bin \
                    ${nonarch_base_libdir}/firmware/tigon/* \
                    ${nonarch_base_libdir}/firmware/vpu_d.bin \
                    ${nonarch_base_libdir}/firmware/vpu_p.bin \
                    ${nonarch_base_libdir}/firmware/vxge/* \
                  "
