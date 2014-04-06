# This flags will be used only by the Marvell arch files compilation.

###################################################################################################
# General definitions
###################################################################################################
CPU_ARCH    = ARM
CHIP        = 88FXX81
VENDOR      = Marvell
ifeq ($(CONFIG_CPU_BIG_ENDIAN),y)
ENDIAN      = BE
else
ENDIAN      = LE
endif

###################################################################################################
# directory structure
###################################################################################################
# Main directory structure
SRC_PATH          = .
PLAT_PATH	  = ../plat-feroceon
PLAT_DRIVERS	  = $(PLAT_PATH)/mv_drivers_lsp
HAL_DIR           = $(PLAT_PATH)/mv_hal
COMMON_DIR        = $(PLAT_PATH)/common
OSSERV_DIR        = $(PLAT_PATH)/linux_oss
LSP_DIR           = $(SRC_PATH)
CONFIG_DIR        = $(LSP_DIR)/config

# HALs
HAL_ETHPHY_DIR    = $(HAL_DIR)/eth-phy
HAL_FLASH_DIR     = $(HAL_DIR)/flash
HAL_RTC_DIR       = $(HAL_DIR)/rtc/ext_rtc
HAL_VOICEBAND     = $(HAL_DIR)/voiceband
HAL_SLIC_DIR      = $(HAL_VOICEBAND)/slic
HAL_DAA_DIR       = $(HAL_VOICEBAND)/daa
HAL_SATA_DIR      = $(HAL_DIR)/sata/CoreDriver/
HAL_QD_DIR        = $(HAL_DIR)/qd-dsdt
HAL_SFLASH_DIR    = $(HAL_DIR)/sflash
HAL_CNTMR_DIR     = $(HAL_DIR)/cntmr
HAL_DRAM_DIR      = $(HAL_DIR)/ddr1_2
HAL_DRAM_ARCH_DIR = $(HAL_DRAM_DIR)/Arch$(CPU_ARCH)
HAL_GPP_DIR       = $(HAL_DIR)/gpp
HAL_TWSI_DIR      = $(HAL_DIR)/twsi
HAL_TWSI_ARCH_DIR = $(SOC_TWSI_DIR)/Arch$(CPU_ARCH)
HAL_UART_DIR      = $(HAL_DIR)/uart
HAL_ETH_NFP_DIR	  = $(HAL_DIR)/eth/nfp
HAL_PCI_DIR       = $(HAL_DIR)/pci
HAL_PCIIF_DIR     = $(HAL_DIR)/pci-if

ifeq ($(CONFIG_MV_INCLUDE_IDMA),y)
HAL_IDMA_DIR      = $(HAL_DIR)/idma
endif
ifeq ($(CONFIG_MV_INCLUDE_PEX),y)
HAL_PEX_DIR       = $(HAL_DIR)/pex
endif
ifeq ($(CONFIG_MV_INCLUDE_TDM),y)
HAL_TDM_DIR       = $(HAL_DIR)/voiceband/tdm
endif
HAL_ETH_DIR       = $(HAL_DIR)/eth/gbe
ifeq ($(CONFIG_MV_INCLUDE_USB),y)
HAL_USB_DIR       = $(HAL_DIR)/usb
endif
ifeq ($(CONFIG_MV_INCLUDE_CESA),y)
HAL_CESA_DIR	  = $(HAL_DIR)/cesa
HAL_CESA_AES_DIR  = $(HAL_DIR)/cesa/AES
endif
ifeq ($(CONFIG_MV_INCLUDE_XOR),y)
HAL_XOR_DIR       = $(HAL_DIR)/xor
endif
ifeq ($(CONFIG_MV_INCLUDE_INTEG_MFLASH),y)
HAL_MFLASH_DIR    = $(HAL_DIR)/mflash
endif
ifeq ($(CONFIG_MV_INCLUDE_SPI),y)
HAL_SPI_DIR       = $(HAL_DIR)/spi
endif
ifeq ($(CONFIG_MV_INCLUDE_AUDIO),y)
HAL_AUDIO_DIR     = $(HAL_DIR)/audio
endif

# Environment components
ORION_FAM_DIR	    = $(LSP_DIR)/orion_family
BOARD_ENV_DIR       = $(ORION_FAM_DIR)/boardEnv
SOC_ENV_DIR         = $(ORION_FAM_DIR)/ctrlEnv
SOC_DEVICE_DIR      = $(ORION_FAM_DIR)/device
SOC_SYS_DIR	    = $(ORION_FAM_DIR)/ctrlEnv/sys
SOC_CPU_DIR	    = $(ORION_FAM_DIR)/cpu

#####################################################################################################
# Include path
###################################################################################################

LSP_PATH_I      = $(TOPDIR)/arch/arm/mach-feroceon-orion
PLAT_PATH_I	= $(TOPDIR)/arch/arm/plat-feroceon

HAL_PATH        = -I$(PLAT_PATH_I)/$(HAL_DIR) -I$(PLAT_PATH_I)/$(HAL_SATA_DIR)
ORION_FAM_PATH  = -I$(LSP_PATH_I)/$(ORION_FAM_DIR)
QD_PATH         = -I$(PLAT_PATH_I)/$(HAL_QD_DIR)/Include  -I$(PLAT_PATH_I)/$(HAL_QD_DIR)/Include/h/msApi 	\
                  -I$(PLAT_PATH_I)/$(HAL_QD_DIR)/Include/h/driver -I$(PLAT_PATH_I)/$(HAL_QD_DIR)/Include/h/platform
                     
COMMON_PATH   	= -I$(PLAT_PATH_I)/$(COMMON_DIR)
 
OSSERV_PATH     = -I$(PLAT_PATH_I)/$(OSSERV_DIR)
LSP_PATH        = -I$(LSP_PATH_I)/$(LSP_DIR)
CONFIG_PATH     = -I$(LSP_PATH_I)/$(CONFIG_DIR)

EXTRA_INCLUDE  	= $(OSSERV_PATH) $(COMMON_PATH) $(HAL_PATH)  $(ORION_FAM_PATH) \
                  $(LSP_PATH) $(CONFIG_PATH)

###################################################################################################
# defines
###################################################################################################
MV_DEFINE = -DMV_LINUX -DMV_CPU_$(ENDIAN) -DMV_$(CPU_ARCH) 

EXTRA_CFLAGS 	+= $(EXTRA_INCLUDE) $(MV_DEFINE)

ifeq ($(CONFIG_MV_GATEWAY),y)
EXTRA_INCLUDE   += $(QD_PATH)
EXTRA_CFLAGS    += -DLINUX  
endif

ifeq ($(CONFIG_MV_CESA_TEST),y)
EXTRA_CFLAGS 	+= -DCONFIG_MV_CESA_TEST
endif

ifeq ($(CONFIG_SATA_DEBUG_ON_ERROR),y)
EXTRA_CFLAGS    += -DMV_LOG_ERROR
endif

ifeq ($(CONFIG_SATA_FULL_DEBUG),y)
EXTRA_CFLAGS    += -DMV_LOG_DEBUG
endif

ifeq ($(CONFIG_MV_SATA_SUPPORT_ATAPI),y)
EXTRA_CFLAGS    += -DMV_SUPPORT_ATAPI
endif

ifeq ($(CONFIG_MV_SATA_ENABLE_1MB_IOS),y)
EXTRA_CFLAGS    += -DMV_SUPPORT_1MBYTE_IOS
endif

ifeq ($(CONFIG_MV88F5182),y)
EXTRA_CFLAGS    += -DMV_88F5182 -DUSB_UNDERRUN_WA
endif

ifeq ($(CONFIG_MV88F5082),y)
EXTRA_CFLAGS    += -DMV_88F5082 -DUSB_UNDERRUN_WA
endif

ifeq ($(CONFIG_MV88F5181),y)
EXTRA_CFLAGS    += -DMV_88F5181
endif

ifeq ($(CONFIG_MV88F5180N),y)
EXTRA_CFLAGS    += -DMV_88F5180N
endif

ifeq ($(CONFIG_MV88F1181),y)
EXTRA_CFLAGS    += -DMV_88F1181
endif

ifeq ($(CONFIG_MV88F1281),y)
EXTRA_CFLAGS    += -DMV_88F1281
endif


ifeq ($(CONFIG_MV88F6082),y)
EXTRA_CFLAGS    += -DMV_88F6082
endif

ifeq ($(CONFIG_MV88F6183),y)
EXTRA_CFLAGS    += -DMV_88F6183
endif


ifeq ($(CONFIG_MV88F5181L),y)
EXTRA_CFLAGS    += -DMV_88F5181L -DUSB_UNDERRUN_WA
endif

ifeq ($(CONFIG_MV88W8660),y)
EXTRA_CFLAGS    += -DMV_88W8660
endif


