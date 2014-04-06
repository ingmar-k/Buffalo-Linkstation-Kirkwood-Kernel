#
# Makefile for the Marvell GPIO Driver
#
 
ifeq ($(CONFIG_ARCH_FEROCEON_KW),y)
	include $(srctree)/arch/arm/mach-feroceon-kw/config/mvRules.mk
endif

ifeq ($(CONFIG_ARCH_FEROCEON_MV78XX0),y)
	include $(srctree)/arch/arm/mach-feroceon-mv78xx0/config/mvRules.mk
endif


obj-y += mv_gpio.o
