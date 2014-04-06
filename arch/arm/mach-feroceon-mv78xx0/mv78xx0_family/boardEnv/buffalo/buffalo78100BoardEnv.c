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

#include <asm/io.h>
#include <asm/byteorder.h>
#include "buffalo78100BoardEnv.h"

//#define DEBUG_ENV
#ifdef DEBUG_ENV
#define DEBUGK(fmt,args...) printk(fmt ,##args)
#else
#define DEBUGK(fmt,args...)
#endif

#if defined(CONFIG_BUFFALO_PLATFORM)

MV_BOARD_GPP_INFO mvtsqInfoBoardGppInfo[MVTSQ_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_GPP_CLASS	devClass, MV_U8	gppPinNum}} */
	{
		{BOARD_GPP_USB_VBUS_EN, 30},
		{BOARD_GPP_USB_VBUS_EN, 31},
		{BOARD_GPP_USB_VBUS_EN, 3},
		{BOARD_GPP_PWRAUTO_SW, 15},
		{BOARD_GPP_MC_IRQ, 14},
		{BOARD_GPP_SATA_HOT, 16},
		{BOARD_GPP_SATA_HOT, 17},
		{BOARD_GPP_SATA_HOT, 18},
		{BOARD_GPP_SATA_HOT, 19},
		{BOARD_GPP_FAN_SELECT, 0},
		{BOARD_GPP_FAN_SELECT, 1},
		{BOARD_GPP_FAN_SELECT, 2},
	};

u32 env_format_version;
EXPORT_SYMBOL(env_format_version);
u32 buffalo_product_id;
EXPORT_SYMBOL(buffalo_product_id);
char buffalo_series_name[BUF_NV_SIZE_SERIES_NAME];
EXPORT_SYMBOL(buffalo_series_name);
char buffalo_product_name[BUF_NV_SIZE_PRODUCT_NAME];
EXPORT_SYMBOL(buffalo_product_name);
//s8 bitHddPower[BUF_NV_SIZE_BIT_HDD_POWER];
//EXPORT_SYMBOL(bitHddPower);
//s8 bitUsbPower[BUF_NV_SIZE_BIT_USB_POWER];
//EXPORT_SYMBOL(bitUsbPower);
//s8 bitInput[BUF_NV_SIZE_BIT_INPUT];
//EXPORT_SYMBOL(bitInput);
//s8 bitOutput[BUF_NV_SIZE_BIT_OUTPUT];
//EXPORT_SYMBOL(bitOutput);
//s8 bitLed[BUF_NV_SIZE_BIT_LED];
//EXPORT_SYMBOL(bitLed);
u8 use_slide_power;
EXPORT_SYMBOL(use_slide_power);

int buffaloBoardInfoInit(void)
{
	extern u32 env_addr;
	extern u32 env_size;

	char *p = ioremap(env_addr + env_size - 1024, 1024);
	if (p) {
		buffalo_product_id = __be32_to_cpu(ioread32(p + 4));
		strncpy(buffalo_series_name,p + 0x10, BUF_NV_SIZE_SERIES_NAME);
		buffalo_series_name[BUF_NV_SIZE_SERIES_NAME - 1] = '\0';
		strncpy(buffalo_product_name,p + 0x30, BUF_NV_SIZE_PRODUCT_NAME);
		buffalo_product_name[BUF_NV_SIZE_PRODUCT_NAME - 1] = '\0';
	}

	if (buffalo_product_id == 0x0 || buffalo_product_id == 0xffffffff)
		buffalo_product_id = 0x0000200B;

	if (strlen(buffalo_series_name) == 0)
		strcpy(buffalo_series_name,"TeraStation");

	if (strlen(buffalo_product_name) == 0)
		strcpy(buffalo_product_name,"TS-XL/R5");

	mvGppPolaritySet(0, 0xffffffff, MV_GPP_IN_ORIGIN);
	mvGppPolaritySet(0, (1 << 14)|(1 << 15)|(1 << 16)|(1 << 17)|(1 << 18)|(1 << 19), MV_GPP_IN_INVERT);

	return 0;
}
EXPORT_SYMBOL(buffaloBoardInfoInit);


/*
 * Move from mach-feroceon-mv78xx0/mv78xx0_family/boardEnv/mvBoardEnvLib.c.
 * This function obsolated linux-feroceon_3_1_3_mv78x_mv88f6323.
 */
MV_32 mvBoardGpioPinNumGet(MV_BOARD_GPP_CLASS class, MV_U32 index)
{
	int i;
	MV_U32 indexFound = 0;

	for (i = 0; i < MVTSQ_BOARD_GPP_INFO_NUM; i++)
		if (mvtsqInfoBoardGppInfo[i].devClass == class) {
			if (indexFound == index)
				return (MV_U32)mvtsqInfoBoardGppInfo[i].gppPinNum;
			else
				indexFound++;
		}

	return MV_ERROR;
}

#endif // CONFIG_BUFFALO_PLATFORM
