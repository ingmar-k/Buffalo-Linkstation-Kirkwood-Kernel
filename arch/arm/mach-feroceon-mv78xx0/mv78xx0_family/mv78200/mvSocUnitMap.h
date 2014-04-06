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
#ifndef mvifmap_h
#define mvifmap_h

typedef enum
{
	UART0=0,
	UART1,
	PEX00,
	PEX10,
	GIGA0,
	GIGA1,
	GIGA2,
	GIGA3,
	SATA,
	XOR,
	IDMA,
	USB0, 
	USB1, 
	USB2, 
	CESA,
	NOR_FLASH,
	NAND_FLASH,
	SPI_FLASH,
	TDM,	
	TWSI0,
	TWSI1,
	RESERVED_0,
	RESERVED_1,
	MAX_UNITS
} MV_SOC_UNIT;

/* binary flags for mvSocUnitMapFillTableFormBitMap */
#define UART0_T0_CPU1		0x0000001
#define UART1_TO_CPU1		0x0000002
#define PEX0_TO_CPU1		0x0000004
#define PEX1_TO_CPU1		0x0000008
#define GIGA0_TO_CPU1		0x0000010
#define GIGA1_TO_CPU1		0x0000020
#define GIGA2_TO_CPU1		0x0000040
#define GIGA3_TO_CPU1		0x0000080
#define SATA_TO_CPU1		0x0000100
#define XOR_TO_CPU1			0x0000200
#define IDMA_TO_CPU1		0x0000400
#define USB0_TO_CPU1		0x0000800
#define USB1_TO_CPU1    	0x0001000
#define USB2_TO_CPU1    	0x0002000
#define CESA_TO_CPU1		0x0004000
#define NOR_TO_CPU1			0x0008000
#define NAND_TO_CPU1		0x0010000
#define SPI_TO_CPU1			0x0020000
#define TDM_TO_CPU1			0x0040000
#define TWSI0_TO_CPU1		0x0080000
#define TWSI1_TO_CPU1		0x0100000 
#define RSRVD0_TO_CPU1		0x0200000 
#define RSRVD1_TO_CPU1		0x0400000 

#define CPU1_DEFAULT_INTERFACE (UART1_TO_CPU1 | PEX1_TO_CPU1 | GIGA2_TO_CPU1 | GIGA3_TO_CPU1 | IDMA_TO_CPU1 | USB1_TO_CPU1 | TWSI1_TO_CPU1) 

typedef struct __MV_RES_MAP 
{
	int cpuId;	
	MV_BOOL mapped;
	char* 	unitName;	
} MV_RES_MAP;

typedef char *(*STRSTR_FUNCPTR)(const char *s1, const char *s2);

MV_BOOL mvSocUnitIsMappedToThisCpu(MV_SOC_UNIT unit);
int mvSocUnitMapGet(MV_SOC_UNIT unit);
MV_VOID mvSocUnitMapSet(MV_SOC_UNIT unit, int cpuId);
MV_BOOL mvSocUnitMapFillTable(char* p, int cpuId, STRSTR_FUNCPTR func);
MV_VOID mvSocUnitMapPrint(MV_8* buf);
MV_BOOL mvSocUnitMapFillTableFormBitMap(MV_U32 flag);
MV_U32 mvSocUnitMapFillFlagFormTable(void);
#endif
