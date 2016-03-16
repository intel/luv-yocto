SRCREV = "deb1d83635089f105c119aec7a949ef6658f0d82"

# linux-firmware add all the licenses for the available firmware binaries.
# We keep only the licenses for the ones we include.

LICENSE = "\
      Firmware-cavium \
    & Firmware-chelsio_firmware \
    & Firmware-e100 \
    & Firmware-kaweth \
    & Firmware-mwl8335 \
    & Firmware-myri10ge_firmware \
    & Firmware-phanfw \
    & Firmware-atmel \
"

NO_GENERIC_LICENSE[Firmware-cavium] = "LICENCE.cavium"
NO_GENERIC_LICENSE[Firmware-chelsio_firmware] = "LICENCE.chelsio_firmware"
NO_GENERIC_LICENSE[Firmware-e100] = "LICENCE.e100"
NO_GENERIC_LICENSE[Firmware-kaweth] = "LICENCE.kaweth"
NO_GENERIC_LICENSE[Firmware-mwl8335] = "LICENCE.mwl8335"
NO_GENERIC_LICENSE[Firmware-myri10ge_firmware] = "LICENCE.myri10ge_firmware"
NO_GENERIC_LICENSE[Firmware-phanfw] = "LICENCE.phanfw"
NO_GENERIC_LICENSE[Firmware-atmel] = "LICENSE.atmel"

LICENSE_${PN} = "\
    Firmware-cavium \
    & Firmware-chelsio_firmware \
    & Firmware-e100 \
    & Firmware-kaweth \
    & Firmware-mwl8335 \
    & Firmware-myri10ge_firmware \
    & Firmware-phanfw \
    & Firmware-atmel \
"

LIC_FILES_CHKSUM = "\
    file://LICENCE.cavium;md5=c37aaffb1ebe5939b2580d073a95daea \
    file://LICENCE.chelsio_firmware;md5=819aa8c3fa453f1b258ed8d168a9d903 \
    file://LICENCE.e100;md5=ec0f84136766df159a3ae6d02acdf5a8 \
    file://LICENCE.kaweth;md5=b1d876e562f4b3b8d391ad8395dfe03f \
    file://LICENCE.mwl8335;md5=9a6271ee0e644404b2ff3c61fd070983 \
    file://LICENCE.myri10ge_firmware;md5=42e32fb89f6b959ca222e25ac8df8fed \
    file://LICENCE.phanfw;md5=954dcec0e051f9409812b561ea743bfa \
    file://LICENSE.atmel;md5=aa74ac0c60595dee4d4e239107ea77a3 \
"

# For now we are only interested in firmware blobs for Ethernet. Thus,
# remove non-Ethernet blobs.
# Also, some drivers uses specific version of firmware binaries. Delete
# those that are not used.
# Lastly, group together the deletion of wireless firmwares as we may
# want to add support for them later.
do_install_append () {
           # storage and Infiniband
           rm -r ${D}/lib/firmware/advansys
           rm -r ${D}/lib/firmware/ene-ub6250
           rm ${D}/lib/firmware/LICENCE.ene_firmware
           rm -r ${D}/lib/firmware/isci
           rm -r ${D}/lib/firmware/qlogic
           rm ${D}/lib/firmware/LICENCE.qla1280
           rm ${D}/lib/firmware/ql2*
           rm ${D}/lib/firmware/LICENCE.qla2xxx

           # USB
           rm ${D}/lib/firmware/r8a779x_usb3_v*.dlmem
           rm ${D}/lib/firmware/LICENCE.r8a779x_usb3

           # media
           rm -r ${D}/lib/firmware/vicam
           rm -r ${D}/lib/firmware/go7007
           rm -r ${D}/lib/firmware/LICENCE.go7007
           rm -r ${D}/lib/firmware/av7110
           rm -r ${D}/lib/firmware/cpia2
           rm -r ${D}/lib/firmware/ttusb-budget
           rm ${D}/lib/firmware/as102_data*
           rm ${D}/lib/firmware/LICENCE.Abilis
           rm ${D}/lib/firmware/dvb-usb-terratec-h5-drxk.fw
           rm ${D}/lib/firmware/cmmb_*
           rm ${D}/lib/firmware/dvb_nova*
           rm ${D}/lib/firmware/isdbt*
           rm ${D}/lib/firmware/sms1xxx-*
           rm ${D}/lib/firmware/LICENCE.siano
           rm ${D}/lib/firmware/dvb-usb-dib*
           rm ${D}/lib/firmware/LICENSE.dib0700
           rm ${D}/lib/firmware/dvb-usb-it*
           rm ${D}/lib/firmware/LICENCE.it913x
           rm ${D}/lib/firmware/dvb-fe-xc*
           rm ${D}/lib/firmware/LICENCE.xc4000
           rm ${D}/lib/firmware/LICENCE.xc5000
           rm ${D}/lib/firmware/LICENCE.xc5000c
           rm ${D}/lib/firmware/f2255usb.bin
           rm ${D}/lib/firmware/lgs8g75.fw
           rm ${D}/lib/firmware/s2250.fw
           rm ${D}/lib/firmware/s2250_loader.fw
           rm ${D}/lib/firmware/s5p*
           rm ${D}/lib/firmware/tlg2300_firmware.bin
           rm ${D}/lib/firmware/v4l-*
           rm ${D}/lib/firmware/tdmb_nova_12mhz.inp

           # rdma
           rm ${D}/lib/firmware/hfi1*
           rm ${D}/lib/firmware/LICENSE.hfi1_firmware

           # microcode
           rm -r ${D}/lib/firmware/amd-ucode
           rm ${D}/lib/firmware/LICENSE.amd-ucode

           # atusb code
           rm -r ${D}/lib/firmware/atusb

           # Bluetooth
           rm -r ${D}/lib/firmware/qca
           rm -r ${D}/lib/firmware/rtl_bt
           rm ${D}/lib/firmware/ath3k-1.fw

           # audio processor
           rm -r ${D}/lib/firmware/dabusb
           rm -r ${D}/lib/firmware/dsp56k
           rm -r ${D}/lib/firmware/emi26
           rm -r ${D}/lib/firmware/emi62
           rm -r ${D}/lib/firmware/ess
           rm -r ${D}/lib/firmware/korg
           rm -r ${D}/lib/firmware/sb16
           rm -r ${D}/lib/firmware/yam
           rm -r ${D}/lib/firmware/yamaha
           rm ${D}/lib/firmware/ctefx.bin
           rm ${D}/lib/firmware/LICENCE.ca0132

           # atm
           rm -r ${D}/lib/firmware/ueagle-atm
           rm ${D}/lib/firmware/LICENCE.ueagle-atm4-firmware
           rm ${D}/lib/firmware/atmsar11.fw

           # USB data acquisition
           rm -r ${D}/lib/firmware/usbdux
           rm ${D}/lib/firmware/usbdux*

           # Intel non-Ethernet firmware
           rm -r ${D}/lib/firmware/intel
           rm ${D}/lib/firmware/LICENCE.adsp_sst
           rm ${D}/lib/firmware/LICENCE.fw_sst_0f28
           rm ${D}/lib/firmware/LICENCE.ibt_firmware
           rm ${D}/lib/firmware/LICENCE.IntcSST2

           # crypto
           rm -r ${D}/lib/firmware/sxg
           rm ${D}/lib/firmware/qat_*
           rm ${D}/lib/firmware/LICENCE.qat_firmware

           # serial devices
           rm -r ${D}/lib/firmware/keyspan
           rm -r ${D}/lib/firmware/keyspan_pda
           rm -r ${D}/lib/firmware/moxa
           rm ${D}/lib/firmware/LICENCE.moxa
           rm -r ${D}/lib/firmware/edgeport
           rm ${D}/lib/firmware/intelliport2.bin
           rm ${D}/lib/firmware/rp2.fw
           rm ${D}/lib/firmware/ti_3410.fw
           rm ${D}/lib/firmware/ti_5052.fw
           rm ${D}/lib/firmware/whiteheat*.fw

           # GPUs
           rm -r ${D}/lib/firmware/i915
           rm -r ${D}/lib/firmware/LICENSE.i915
           rm -r ${D}/lib/firmware/nvidia
           rm ${D}/lib/firmware/LICENCE.nvidia
           rm -r ${D}/lib/firmware/radeon
           rm ${D}/lib/firmware/LICENSE.radeon
           rm -r ${D}/lib/firmware/amdgpu
           rm ${D}/lib/firmware/LICENSE.amdgpu
           rm -r ${D}/lib/firmware/matrox
           rm -r ${D}/lib/firmware/r128

           # TI queue manager
           rm -r ${D}/lib/firmware/ti-keystone
           rm ${D}/lib/firmware/LICENCE.ti-keystone

           # wimax
           rm ${D}/lib/firmware/i2400m*
           rm ${D}/lib/firmware/LICENCE.i2400m
           rm ${D}/lib/firmware/i6050*

           # wireless modem
           rm ${D}/lib/firmware/mts_*

           # wireless drivers. Keep these together as we may
           # want to add support for them in the future
           rm ${D}/lib/firmware/agere*
           rm ${D}/lib/firmware/LICENCE.agere
           rm -r ${D}/lib/firmware/carl9170-1.fw
           rm -r ${D}/lib/firmware/libertas
           rm ${D}/lib/firmware/LICENCE.OLPC
           rm ${D}/lib/firmware/sd8686.bin
           rm ${D}/lib/firmware/sd8686_helper.bin
           rm -r ${D}/lib/firmware/ath10k
           rm ${D}/lib/firmware/LICENSE.QualcommAtheros_ath10k
           rm -r ${D}/lib/firmware/ath9k_htc
           rm ${D}/lib/firmware/LICENCE.open-ath9k-htc-firmware
           rm -r ${D}/lib/firmware/mwl8k
           rm -r ${D}/lib/firmware/mwlwifi
           rm -r ${D}/lib/firmware/rtlwifi
           rm -r ${D}/lib/firmware/LICENCE.rtlwifi_firmware.txt
           rm -r ${D}/lib/firmware/ath6k
           rm -r ${D}/lib/firmware/RTL8192E
           rm -r ${D}/lib/firmware/mrvl
           rm -r ${D}/lib/firmware/LICENCE.Marvell
           rm -r ${D}/lib/firmware/brcm
           rm ${D}/lib/firmware/LICENCE.broadcom_bcm43xx
           rm -r ${D}/lib/firmware/ar3k
           rm ${D}/lib/firmware/LICENSE.QualcommAtheros_ar3k
           rm -r ${D}/lib/firmware/ti-connectivity
           rm -r ${D}/lib/firmware/LICENCE.ti-connectivity
           rm -r ${D}/lib/firmware/LICENCE.wl1251
           rm ${D}/lib/firmware/wl1*.bin
           rm ${D}/lib/firmware/TIInit_7.2.31.bts
           rm ${D}/lib/firmware/htc_7010.fw
           rm ${D}/lib/firmware/htc_9271.fw
           rm ${D}/lib/firmware/LICENCE.atheros_firmware
           rm ${D}/lib/firmware/iwlwifi*
           rm ${D}/lib/firmware/LICENCE.iwlwifi_firmware
           rm ${D}/lib/firmware/lbtf_usb.bin
           rm ${D}/lib/firmware/mt7601u.bin
           rm ${D}/lib/firmware/mt7650.bin
           rm ${D}/lib/firmware/LICENCE.ralink_a_mediatek_company_firmware
           rm ${D}/lib/firmware/ar5523.bin
           rm ${D}/lib/firmware/rsi_91x.fw
           rm ${D}/lib/firmware/rt2*
           rm ${D}/lib/firmware/rt3*
           rm ${D}/lib/firmware/rt7*
           rm ${D}/lib/firmware/sdd_sagrad_1091_1098.bin
           rm ${D}/lib/firmware/vntwusb.fw
           rm ${D}/lib/firmware/LICENCE.via_vt6656
           rm ${D}/lib/firmware/wsm_22.bin
           rm ${D}/lib/firmware/LICENCE.cw1200

           # unused individual firmware pieces
           rm ${D}/lib/firmware/ar7010*
           rm ${D}/lib/firmware/ar9170*
           rm ${D}/lib/firmware/ar9271*
           rm ${D}/lib/firmware/ctspeq.bin
           rm ${D}/lib/firmware/TDA7706*
           rm ${D}/lib/firmware/LICENCE.tda7706-firmware.txt
           rm ${D}/lib/firmware/tr_smctr.bin

           # keep only the version used by Linux v4.4: 3.2.3.0
           rm ${D}/lib/firmware/cbfw-3.2.[!3]*

           # keep only the FW versions used by drivers in Linux v4.4
           # v7.12.30.0
           rm ${D}/lib/firmware/bnx2x/bnx2x-e*-6*
           rm ${D}/lib/firmware/bnx2x/bnx2x-e*-7.1[0,1,3]*
           rm ${D}/lib/firmware/bnx2x/bnx2x-e*-7.[0,2,8]*
           rm ${D}/lib/firmware/bnx2x-*

           # keep only the FW versions used by drivers in Linux v4.4
           # bnx2-mips-06-6.2.3.fw and bnx2-mips-09-6.2.1b.fw
           rm ${D}/lib/firmware/bnx2/bnx2-rv2p*
           rm ${D}/lib/firmware/bnx2/bnx2-mips-0?-[4,5]*
           rm ${D}/lib/firmware/bnx2/bnx2-mips-0?-6.0*
           rm ${D}/lib/firmware/bnx2/bnx2-mips-06-6.2.[!3]*
           rm ${D}/lib/firmware/bnx2/bnx2-mips-09-6.2.1[!b]*

           # keep only the FW versions used by drivers in Linux v4.4
           # v7.12.0
           rm ${D}/lib/firmware/cxgb3/t3fw-7.[!1]*
           rm ${D}/lib/firmware/cxgb3/t3fw-7.1[!2]*
           ls ${D}/lib/firmware/cxgb3

           # keep only the versions used by drivers in Linux v4.4
           rm ${D}/lib/firmware/qed/qed_init_values_zipped-8.7.3.0.bin

           # keep only drivers used in Linux v4.4
           rm ${D}/lib/firmware/3com/3C359.bin
           # keep v3.2.5.1
           rm ${D}/lib/firmware/ct2fw-3.2.[!5]*
           rm ${D}/lib/firmware/ctfw-3.2.[!5]*
           rm ${D}/lib/firmware/myri10ge_eth_big_z8e.dat
           rm ${D}/lib/firmware/myri10ge_ethp_big_z8e.dat
           rm ${D}/lib/firmware/myri10ge_rss_eth_big_z8e.dat
           rm ${D}/lib/firmware/myri10ge_rss_ethp_big_z8e.dat
}

# linux-firmware defines some alternatives for specific wireless
# firmware binaries. We don't need them for now.
python __anonymous() {
    d.delVar('ALTERNATIVE_linux-firmware-bcm4334')
    d.delVar('ALTERNATIVE_TARGET_linux-firmware-bcm4334')
    d.delVar('ALTERNATIVE_linux-firmware-bcm43340')
    d.delVar('ALTERNATIVE_TARGET_linux-firmware-bcm43340')
    d.delVar('ALTERNATIVE_linux-firmware-bcm4354')
    d.delVar('ALTERNATIVE_TARGET_linux-firmware-bcm4354')
    d.delVar('ALTERNATIVE_linux-firmware-bcm4329')
    d.delVar('ALTERNATIVE_TARGET_linux-firmware-bcm4329')
    d.delVar('ALTERNATIVE_linux-firmware-bcm4330')
    d.delVar('ALTERNATIVE_TARGET_linux-firmware-bcm4330')
    d.delVar('ALTERNATIVE_linux-firmware-bcm4339')
    d.delVar('ALTERNATIVE_TARGET_linux-firmware-bcm4339')
    # d.delVarFlag('ALTERNATIVE_LINK_NAME', 'brcmfmac-sdio.bin')
}

# linux-firmware has prepend to populate_packages to make it depend
# on all its split-out packages. We don't want that as all of those
# packages are for firmware blobs for wireless networking. We currently
# don't include them in our images. Thus, we make linux-firmware depend
# only in linux-firmware-license. The function populate_packages is part
# of PACKAGESPLITFUNCS. Thus, we simply append a new function.
python fixup_populate_packages () {
    d.setVar('RDEPENDS_linux-firmware', 'linux-firmware-license')
}

PACKAGESPLITFUNCS_append = " fixup_populate_packages "
