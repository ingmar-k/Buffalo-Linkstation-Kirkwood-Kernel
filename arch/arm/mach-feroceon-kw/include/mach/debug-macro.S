/*
 * arch/arm/mach-kirkwood/include/mach/debug-macro.S
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

//#include <mach/bridge-regs.h>
#define KIRKWOOD_REGS_PHYS_BASE		0xf1000000
#define KIRKWOOD_REGS_VIRT_BASE		0xfed00000
//#define KIRKWOOD_REGS_SIZE		SZ_1M

	.macro	addruart, rp, rv, tmp
	ldr	\rp, =KIRKWOOD_REGS_PHYS_BASE
	ldr	\rv, =KIRKWOOD_REGS_PHYS_BASE
	orr	\rp, \rp, #0x00012000
	orr	\rv, \rv, #0x00012000
	.endm

#define UART_SHIFT	2
#include <asm/hardware/debug-8250.S>
