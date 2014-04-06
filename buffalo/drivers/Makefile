obj-$(CONFIG_BUFFALO_PLATFORM)		+= buffalo_proc_entry.o

ifeq ($(CONFIG_X86), y)
obj-$(CONFIG_BUFFALO_PLATFORM)		+= buffalocore.o
endif

ifeq ($(CONFIG_ARM), y)
ifeq ($(CONFIG_ARCH_FEROCEON_ORION), y)
include $(srctree)/arch/arm/mach-feroceon-orion/config/mvRules.mk
endif
ifeq ($(CONFIG_ARCH_FEROCEON_KW), y)
include $(srctree)/arch/arm/mach-feroceon-kw/config/mvRules.mk
endif
ifeq ($(CONFIG_ARCH_FEROCEON_MV78XX0), y)
include $(srctree)/arch/arm/mach-feroceon-mv78xx0/config/mvRules.mk
endif
ifeq ($(CONFIG_ARCH_ARMADA370), y)
include $(srctree)/arch/arm/mach-armada370/config/mvRules.mk
endif
ifeq ($(CONFIG_ARCH_ARMADA_XP), y)
include $(srctree)/arch/arm/mach-armadaxp/config/mvRules.mk
endif
ifeq ($(CONFIG_ARCH_FEROCEON_MV78XX0), y)
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloUart.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloGpio.o
endif
ifeq ($(CONFIG_ARCH_FEROCEON_KW), y)
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloUart.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloGpio.o
endif
endif

obj-$(CONFIG_BUFFALO_PLATFORM)		+= kernevntProc.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= kernevnt.o

ifeq ($(CONFIG_X86), y)
obj-$(CONFIG_BUFFALO_PLATFORM)		+= buffalo_upsdrv.o
drivers-$(CONFIG_BUFFALO_MATSU_PLATFORM) += buffalo/plat-matsu/
drivers-$(CONFIG_BUFFALO_KIRI_PLATFORM) += buffalo/plat-kiri/
endif

ifeq ($(CONFIG_ARM), y)
ifeq ($(CONFIG_ARCH_FEROCEON_MV78XX0), y)
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloGpioDriver.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloInterruptDriver.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= buffalo_miconctl_v2.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= buffalo_upsdrv.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= buffalocore.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloSataHotplug.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= plat-feroceon/
endif
ifeq ($(CONFIG_ARCH_FEROCEON_KW), y)
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloGpioDriver.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloInterruptDriver.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= buffalocore.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloWol.o
obj-$(CONFIG_BUFFALO_USE_MICON)		+= buffalo_miconctl_v2.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= BuffaloSataHotplug.o
obj-$(CONFIG_BUFFALO_PLATFORM)		+= plat-feroceon/
endif
ifeq ($(CONFIG_ARCH_ARMADA370), y)
obj-$(CONFIG_BUFFALO_PLATFORM)		+= buffalocore.o
endif
ifeq ($(CONFIG_ARCH_ARMADA_XP), y)
obj-$(CONFIG_BUFFALO_PLATFORM)		+= buffalocore.o
endif
endif
