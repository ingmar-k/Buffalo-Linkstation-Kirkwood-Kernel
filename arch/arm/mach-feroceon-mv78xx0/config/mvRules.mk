# This flags will be used only by the Marvell arch files compilation.

###################################################################################################
# General definitions
###################################################################################################
CPU_ARCH    = ARM
CHIP        = MV78XX0
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
HAL_DRAM_DIR      = $(HAL_DIR)/ddr2
HAL_DRAM_SPD_DIR  = $(HAL_DRAM_DIR)/spd
HAL_DRAM_ARCH_DIR = $(HAL_DRAM_DIR)/Arch$(CPU_ARCH)
HAL_GPP_DIR       = $(HAL_DIR)/gpp
HAL_TWSI_DIR      = $(HAL_DIR)/twsi
HAL_UART_DIR      = $(HAL_DIR)/uart
HAL_ETH_DIR       = $(HAL_DIR)/eth/gbe
HAL_ETH_NFP_DIR	  = $(HAL_DIR)/eth/nfp
HAL_CPU_DIR       = $(HAL_DIR)/cpu

ifeq ($(CONFIG_MV_INCLUDE_PEX),y)
HAL_PEX_DIR       = $(HAL_DIR)/pex
endif
ifeq ($(CONFIG_MV_INCLUDE_TDM),y)
HAL_TDM_DIR       = $(HAL_DIR)/voiceband/tdm
endif
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

ifeq ($(CONFIG_MV_INCLUDE_IDMA),y)
HAL_IDMA_DIR       = $(HAL_DIR)/idma
endif

ifeq ($(CONFIG_MV_INCLUDE_SPI),y)
HAL_SPI_DIR       = $(HAL_DIR)/spi
endif
ifeq ($(CONFIG_MV_INCLUDE_AUDIO),y)
HAL_AUDIO_DIR     = $(HAL_DIR)/audio
endif
ifeq ($(CONFIG_MV_INCLUDE_TS),y)
HAL_TS_DIR      = $(HAL_DIR)/ts
endif

# Environment components
FAM_DIR	    	    = $(LSP_DIR)/mv78xx0_family
SOC_DEVICE_DIR      = $(FAM_DIR)/device
SOC_CPU_DIR         = $(FAM_DIR)/cpu
BOARD_ENV_DIR       = $(FAM_DIR)/boardEnv
SOC_ENV_DIR         = $(FAM_DIR)/ctrlEnv
SOC_SYS_DIR	    = $(FAM_DIR)/ctrlEnv/sys
SOC_DUAL_CPU_DIR    = $(FAM_DIR)/mv78200

#####################################################################################################
# Include path
###################################################################################################

LSP_PATH_I      = $(srctree)/arch/arm/mach-feroceon-mv78xx0
PLAT_PATH_I	= $(srctree)/arch/arm/plat-feroceon

HAL_PATH        = -I$(PLAT_PATH_I)/$(HAL_DIR) -I$(PLAT_PATH_I)/$(HAL_SATA_DIR)
FAM_PATH  = 	  -I$(LSP_PATH_I)/$(FAM_DIR)
QD_PATH         = -I$(PLAT_PATH_I)/$(HAL_QD_DIR)/Include  -I$(PLAT_PATH_I)/$(HAL_QD_DIR)/Include/h/msApi 	\
                  -I$(PLAT_PATH_I)/$(HAL_QD_DIR)/Include/h/driver -I$(PLAT_PATH_I)/$(HAL_QD_DIR)/Include/h/platform
                     
COMMON_PATH   	= -I$(PLAT_PATH_I)/$(COMMON_DIR)
 
OSSERV_PATH     = -I$(PLAT_PATH_I)/$(OSSERV_DIR)
LSP_PATH        = -I$(LSP_PATH_I)/$(LSP_DIR)
CONFIG_PATH     = -I$(LSP_PATH_I)/$(CONFIG_DIR)

EXTRA_INCLUDE  	= $(OSSERV_PATH) $(COMMON_PATH) $(FAM_PATH) $(HAL_PATH)  \
                  $(LSP_PATH) $(CONFIG_PATH)


ifneq (${CONFIG_BUFFALO_PLATFORM},)
EXTRA_INCLUDE	+= -I$(PLAT_PATH_I)/$(PLAT_DRIVERS) -I${srctree}/buffalo/drivers
endif

###################################################################################################
# defines
###################################################################################################
MV_DEFINE = -DMV_LINUX -DMV_CPU_$(ENDIAN) -DMV_$(CPU_ARCH) 

EXTRA_CFLAGS 	+= $(EXTRA_INCLUDE) $(MV_DEFINE)

ifeq ($(CONFIG_MV_GATEWAY),y)
EXTRA_INCLUDE   += $(QD_PATH)
EXTRA_CFLAGS    += -DLINUX  
endif

ifeq ($(CONFIG_VIRTUAL_BRIDGE_SUPPORT),y)
EXTRA_CFLAGS 	+=-DVIRTUAL_BRIDGE_SUPPORT
endif

ifeq ($(CONFIG_USB_PHY_OVERRIDE_SETTINGS),n)
EXTRA_CFLAGS    += -DMV_USB_PHY_DONT_OVERRIDE
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

ifeq ($(CONFIG_MV78XX0),y)
EXTRA_CFLAGS    += -DMV78XX0
endif

ifeq ($(CONFIG_MV78200),y)
EXTRA_CFLAGS    += -DMV78200
endif

ifeq ($(CONFIG_MV632X),y)
EXTRA_CFLAGS    += -DMV78200 -DMV632X
endif

ifeq ($(CONFIG_MV6323),y)
EXTRA_CFLAGS    += -DMV6323
endif


ifeq ($(CONFIG_MV6321),y)
EXTRA_CFLAGS    += -DMV6321
endif

ifeq ($(CONFIG_MV6322),y)
EXTRA_CFLAGS    += -DMV6322
endif


ifeq ($(CONFIG_MV76100),y)
EXTRA_CFLAGS    += -DMV76100
endif


ifeq ($(CONFIG_DB_MV78XX0),y)
EXTRA_CFLAGS    += -DDB_MV78XX0
endif

ifeq ($(CONFIG_RD_MV78XX0_AMC),y)
EXTRA_CFLAGS    += -DRD_MV78XX0_AMC
endif

ifeq ($(CONFIG_RD_MV78XX0_MASA),y)
EXTRA_CFLAGS    += -DRD_MV78XX0_MASA
endif

ifeq ($(CONFIG_RD_MV78XX0_H3C),y)
EXTRA_CFLAGS    += -DRD_MV78XX0_H3C
endif

ifeq ($(CONFIG_MV78XX0_Z0),y)
EXTRA_CFLAGS    += -DMV78XX0_Z0 -DMV_BRIDGE_SYNC_REORDER
endif

EXTRA_AFLAGS 	+= $(EXTRA_CFLAGS)
