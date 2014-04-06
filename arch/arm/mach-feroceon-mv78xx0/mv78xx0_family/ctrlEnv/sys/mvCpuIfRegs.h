/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/


#ifndef __INCmvCpuIfRegsh
#define __INCmvCpuIfRegsh

/****************************************/
/* ARM Control and Status Registers Map */
/****************************************/
#define CPU_CONFIG_REG(cpu)			(AHB_TO_MBUS_BASE(cpu) + 0x100)
#define CPU_CTRL_STAT_REG(cpu)			(AHB_TO_MBUS_BASE(cpu) + 0x104)
#define CPU_AHB_MBUS_CAUSE_INT_REG(cpu)		(AHB_TO_MBUS_BASE(cpu) + 0x110)
#define CPU_AHB_MBUS_MASK_INT_REG(cpu)		(AHB_TO_MBUS_BASE(cpu) + 0x114)
#define CPU_PM_CTRL_REG(cpu)			(AHB_TO_MBUS_BASE(cpu) + 0x11C)
#define CPU_TIMING_ADJUST_REG(cpu)		(AHB_TO_MBUS_BASE(cpu) + 0x120)
#define CPU_L1_RAM_TIMING0_REG(cpu)		(AHB_TO_MBUS_BASE(cpu) + 0x128)
#define CPU_L1_RAM_TIMING1_REG(cpu)		(AHB_TO_MBUS_BASE(cpu) + 0x12C)
#define CPU_MBUS_TIMEOUT_REG(cpu)		(AHB_TO_MBUS_BASE(cpu) + 0x130)
#define CPU_L2_RAM_TIMING0_REG(cpu)		(AHB_TO_MBUS_BASE(cpu) + 0x134)
#define CPU_L2_RAM_TIMING1_REG(cpu)		(AHB_TO_MBUS_BASE(cpu) + 0x138)
#define CPU_MEMORY_PM_CTRL_REG(cpu)		(AHB_TO_MBUS_BASE(cpu) + 0x140)
#define CPU_L2_RAM_PM_CTRL_REG(cpu)		(AHB_TO_MBUS_BASE(cpu) + 0x144)

/* ARM Configuration register */
/* CPU_CONFIG_REG (CCR) */

/* Reset vector location */
#define CCR_VEC_INIT_LOC_OFFS			1
#define CCR_VEC_INIT_LOC_MASK			BIT1
/* reset at 0x00000000 */
#define CCR_VEC_INIT_LOC_0000			(0 << CCR_VEC_INIT_LOC_OFFS)
/* reset at 0xFFFF0000 */
#define CCR_VEC_INIT_LOC_FF00			(1 << CCR_VEC_INIT_LOC_OFFS)

#define CCR_AHB_ERROR_PROP_OFFS			2
#define CCR_AHB_ERROR_PROP_MASK			BIT2
/* Erros are not propogated to AHB */
#define CCR_AHB_ERROR_PROP_NO_INDICATE	(0 << CCR_AHB_ERROR_PROP_OFFS)
/* Erros are propogated to AHB */
#define CCR_AHB_ERROR_PROP_INDICATE		(1 << CCR_AHB_ERROR_PROP_OFFS)


#define CCR_ENDIAN_INIT_OFFS			3
#define CCR_ENDIAN_INIT_MASK			BIT3
#define CCR_ENDIAN_INIT_LITTLE			(0 << CCR_ENDIAN_INIT_OFFS)
#define CCR_ENDIAN_INIT_BIG			(1 << CCR_ENDIAN_INIT_OFFS)

#define CCR_CPU_ID_OFFS				4
#define CCR_CPU_ID_MASK				BIT4
#define CCR_ARM_CPU_ID				(0 << CCR_CPU_ID_OFFS)
#define CCR_MRVL_CPU_ID				(1 << CCR_CPU_ID_OFFS)


#define CCR_MMU_DISABLED_OFFS			5			
#define CCR_MMU_DISABLED_MASK			(1 << CCR_MMU_DISABLED_OFFS)
#define CCR_MMU_ENABLED				(0 << CCR_MMU_DISABLED_OFFS)
#define CCR_MMU_DISABLED			(1 << CCR_MMU_DISABLED_OFFS)

#define CCR_CPU_2_AHB_TICK_DRV_OFFS		8
#define CCR_CPU_2_AHB_TICK_DRV_MASK		(0xF << CCR_CPU_2_AHB_TICK_DRV_OFFS)
#define CCR_CPU_2_AHB_TICK_SMPL_OFFS		12
#define CCR_CPU_2_AHB_TICK_SMPL_MASK		(0xF << CCR_CPU_2_AHB_TICK_SMPL_OFFS)
#define CCR_ICACH_PREF_BUF_ENABLE		BIT16
#define CCR_DCACH_PREF_BUF_ENABLE		BIT17

#define CCR_AHB_ERROR_PROP_OFFS			2
#define CCR_AHB_ERROR_PROP_MASK			BIT2
/* Erros are not propogated to AHB */
#define CCR_AHB_ERROR_PROP_NO_INDICATE	(0 << CCR_AHB_ERROR_PROP_OFFS)
/* Erros are propogated to AHB */
#define CCR_AHB_ERROR_PROP_INDICATE		(1 << CCR_AHB_ERROR_PROP_OFFS)

/* ARM Control and Status register */
/* CPU_CTRL_STAT_REG (CCSR) */


/*
This is used to block PCI express\PCI from access Socrates/Feroceon GP
while ARM boot is still in progress
*/

#define CCSR_PEX0_ENABLE					BIT0
#define CCSR_PEX1_ENABLE					BIT1
#define CCSR_ARM_RESET						BIT3
#define CCSR_SELF_INT						BIT2
#define CCSR_BIG_ENDIAN						BIT15
#define CCSR_L2WT                           BIT17
#define CCSR_DDR_RD_WIDTH					BIT18
#define CCSR_DDR_WR_WIDTH					BIT19
#define CCSR_L2_CACHE_SIZE					BIT21
#define CCSR_L2_PARITY_PROTECTION			BIT24

/* Mbus-L to Mbus Bridge Interrupt Cause Register*/
/* CPU_AHB_MBUS_CAUSE_INT_REG (CAMCIR) */

#define CMMCIR_ARM_SELF_INT				BIT0
#define CMMCIR_ARM_TIMER0_INT_REQ			BIT1
#define CMMCIR_ARM_TIMER1_INT_REQ			BIT2
#define CMMCIR_ARM_WD_TIMER_INT_REQ			BIT3
#define CMMCIR_ARM_TIMER2_INT_REQ			BIT6
#define CMMCIR_ARM_TIMER3_INT_REQ			BIT7


/* Mbus-L to Mbus Bridge Interrupt Mask Register*/
/* CPU_AHB_MBUS_MASK_INT_REG (CAMMIR) */

#define CMMCIR_ARM_SELF_INT_OFFS	0
#define CMMCIR_ARM_SELF_INT_MASK	BIT0
#define CMMCIR_ARM_SELF_INT_EN		(1 << CMMCIR_ARM_SELF_INT_OFFS)
#define CMMCIR_ARM_SELF_INT_DIS		(0 << CMMCIR_ARM_SELF_INT_OFFS)


#define CMMCIR_ARM_TIMER0_INT_REQ_OFFS	1
#define CMMCIR_ARM_TIMER0_INT_REQ_MASK	BIT1
#define CMMCIR_ARM_TIMER0_INT_REQ_EN	(1 << CMMCIR_ARM_TIMER0_INT_REQ_OFFS) 
#define CMMCIR_ARM_TIMER0_INT_REQ_DIS	(0 << CMMCIR_ARM_TIMER0_INT_REQ_OFFS)

#define CMMCIR_ARM_TIMER1_INT_REQ_OFFS	2
#define CMMCIR_ARM_TIMER1_INT_REQ_MASK	BIT2
#define CMMCIR_ARM_TIMER1_INT_REQ_EN	(1 << CMMCIR_ARM_TIMER1_INT_REQ_OFFS) 
#define CMMCIR_ARM_TIMER1_INT_REQ_DIS	(0 << CMMCIR_ARM_TIMER1_INT_REQ_OFFS) 

#define CMMCIR_ARM_TIMER2_INT_REQ_OFFS	6
#define CMMCIR_ARM_TIMER2_INT_REQ_MASK	BIT6
#define CMMCIR_ARM_TIMER2_INT_REQ_EN	(1 << CMMCIR_ARM_TIMER0_INT_REQ_OFFS) 
#define CMMCIR_ARM_TIMER2_INT_REQ_DIS	(0 << CMMCIR_ARM_TIMER0_INT_REQ_OFFS)

#define CMMCIR_ARM_TIMER3_INT_REQ_OFFS	7
#define CMMCIR_ARM_TIMER3_INT_REQ_MASK	BIT7
#define CMMCIR_ARM_TIMER3_INT_REQ_EN	(1 << CMMCIR_ARM_TIMER1_INT_REQ_OFFS) 
#define CMMCIR_ARM_TIMER3_INT_REQ_DIS	(0 << CMMCIR_ARM_TIMER1_INT_REQ_OFFS) 


#define CMMCIR_ARM_WD_TIMER_INT_REQ_OFFS 3
#define CMMCIR_ARM_WD_TIMER_INT_REQ_MASK BIT3
#define CMMCIR_ARM_WD_TIMER_INT_REQ_EN	 (1 << CMMCIR_ARM_WD_TIMER_INT_REQ_OFFS) 
#define CMMCIR_ARM_WD_TIMER_INT_REQ_DIS	 (0 << CMMCIR_ARM_WD_TIMER_INT_REQ_OFFS) 


/*******************************************/
/* Main Interrupt Controller Registers Map */
/*******************************************/

#define CPU_INT_ERROR_REG(cpu) 	    		(AHB_TO_MBUS_BASE(cpu) + 0x200)
#define CPU_INT_LOW_REG(cpu) 	    		(AHB_TO_MBUS_BASE(cpu) + 0x204)
#define CPU_INT_HIGH_REG(cpu) 	    		(AHB_TO_MBUS_BASE(cpu) + 0x208)
#define CPU_INT_MASK_ERROR_REG(cpu)	    	(AHB_TO_MBUS_BASE(cpu) + 0x20C)
#define CPU_INT_MASK_LOW_REG(cpu) 	    	(AHB_TO_MBUS_BASE(cpu) + 0x210)
#define CPU_INT_MASK_HIGH_REG(cpu) 	    	(AHB_TO_MBUS_BASE(cpu) + 0x214)
#define CPU_INT_SELECT_CAUSE_REG(cpu) 		(AHB_TO_MBUS_BASE(cpu) + 0x218)


/*******************************************/
/* Power Management control			   */
/*******************************************/

#define CPMCR_CLOCK_REALIGN			BIT0
#define CPMCR_GBE_POWER_UP(num)			(BIT1 << (num))
#define CPMCR_PEX_POWER_UP(num, bar)		((BIT5 << (bar)) << ((num)*4))
#define CPMCR_SATA_PHY_POWER_UP(num)		(BIT13 << (num*2))
#define CPMCR_SATA_HC_POWER_UP(num)		(BIT14 << (num*2))
#define CPMCR_USB_POWER_UP(num)			(BIT17 << (num))
#define CPMCR_IDMA_POWER_UP			BIT20
#define CPMCR_XOR_POWER_UP			BIT21
#define CPMCR_CRYPTO_POWER_UP			BIT22
#define CPMCR_DEVICE_POWER_UP			BIT23




/*******************************************/
/* ARM Doorbell Registers Map			   */
/*******************************************/

#define CPU_HOST_TO_ARM_DRBL_REG		(AHB_TO_MBUS_BASE(cpu) + 0x400)
#define CPU_HOST_TO_ARM_MASK_REG		(AHB_TO_MBUS_BASE(cpu) + 0x404)
#define CPU_ARM_TO_HOST_DRBL_REG		(AHB_TO_MBUS_BASE(cpu) + 0x408)
#define CPU_ARM_TO_HOST_MASK_REG		(AHB_TO_MBUS_BASE(cpu) + 0x40C)


#ifdef MV_CPU_LE
#define CL2CR_L2_ECC_EN_OFFS			24
#define CL2CR_L2_WT_MODE_OFFS			17
#else
#define CL2CR_L2_ECC_EN_OFFS			0
#define CL2CR_L2_WT_MODE_OFFS			9
#endif

#define CL2CR_L2_ECC_EN_MASK		(1 << CL2CR_L2_ECC_EN_OFFS)
#define CL2CR_L2_WT_MODE_MASK		(1 << CL2CR_L2_WT_MODE_OFFS)

#define CPU_L2_CONFIG_REG			0x20104
#define CPU_CORE1_OFFSET       			0x4000


#endif /* __INCmvCpuIfRegsh */

