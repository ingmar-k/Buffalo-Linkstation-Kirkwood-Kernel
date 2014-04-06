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

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cpu/mvCpu.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "mv78200/mvSocUnitMap.h"


static MV_RES_MAP mv_res_table[] = {
	{0, MV_FALSE, "uart0"},
	{1, MV_FALSE, "uart1"},
	{0, MV_FALSE, "pcie0"},
	{0, MV_FALSE, "pcie1"},
	{0, MV_FALSE, "egiga0"},
	{0, MV_FALSE, "egiga1"},
	{1, MV_FALSE, "egiga2"},
	{1, MV_FALSE, "egiga3"},
	{0, MV_FALSE, "sata"},
	{0, MV_FALSE, "xor"},
	{0, MV_FALSE, "idma"},
	{0, MV_FALSE, "usb0"},
	{0, MV_FALSE, "usb1"},
	{0, MV_FALSE, "usb2"},
	{0, MV_FALSE, "cesa"},
	{0, MV_FALSE, "nor"},
	{0, MV_FALSE, "nand"},
	{0, MV_FALSE, "spi"},
	{0, MV_FALSE, "tdm"},
	{0, MV_FALSE, "twsi0"},
	{0, MV_FALSE, "twsi1"},
	{0, MV_FALSE, ""},
	{0, MV_FALSE, ""},
	{-1, MV_FALSE, 0},
	};




int mvSocUnitMapGet(MV_SOC_UNIT unit)
{
	return mv_res_table[unit].cpuId;
}

MV_BOOL mvSocUnitIsMappedToThisCpu(MV_SOC_UNIT unit)
{
	return (mvSocUnitMapGet(unit) == whoAmI());
}


MV_VOID mvSocUnitMapSet(MV_SOC_UNIT unit, int cpuId)
{
	if (MV_TRUE == mv_res_table[unit].mapped)
	{
		mvOsPrintf("Warning! Unit %s is already mapped to CPU%d\n",
			   mv_res_table[unit].unitName, 
			   mv_res_table[unit].cpuId);
		return;
	}
	mv_res_table[unit].cpuId = cpuId;
	mv_res_table[unit].mapped = MV_TRUE;
}


MV_BOOL mvSocUnitMapFillTable(char* p, int cpuId, STRSTR_FUNCPTR strstr_func)
{
	int i;
	char* tmp;
	const char *syntaxErr = "mvSocUnitMapFillTable: syntax error (%s)\n";
	for (i = 0; mv_res_table[i].cpuId != -1; i++) {		
		char *t = mv_res_table[i].unitName;
		int len;
		if (t[0] == 0) {/*empty string*/
			continue;
		}
		tmp = strstr_func(p, t);
		if (!tmp) continue;
		/*Look for delimiter*/
		if (tmp > p) {
			if (tmp[-1] != ' ' && tmp[-1] != ',') {
				mvOsPrintf(syntaxErr, mv_res_table[i].unitName);
				continue;
			}
		}
		while (*t++ != '\0');
		len = t - mv_res_table[i].unitName - 1;
		if (tmp[len] != ' ' && tmp[len] != ',' && tmp[len] != '\0') {
			mvOsPrintf(syntaxErr, mv_res_table[i].unitName);
			continue;
		}
			mvSocUnitMapSet(i, cpuId);
	}
	return MV_TRUE;
}

MV_BOOL mvSocUnitMapFillTableFormBitMap(MV_U32 flag)
{
	int i,bit,cpuId;
	for (i = 0; mv_res_table[i].cpuId != -1; i++) 
	{
		switch (i)
		{
		case UART0:  bit=UART0_T0_CPU1;   break;        
		case UART1:  bit=UART1_TO_CPU1;   break;       
		case PEX00:  bit=PEX0_TO_CPU1;    break;       
		case PEX10:  bit=PEX1_TO_CPU1;    break;       
		case GIGA0:  bit=GIGA0_TO_CPU1;   break;       
		case GIGA1:  bit=GIGA1_TO_CPU1;   break;       
		case GIGA2:  bit=GIGA2_TO_CPU1;   break;       
		case GIGA3:  bit=GIGA3_TO_CPU1;   break;       
		case SATA:   bit=SATA_TO_CPU1;    break;        
		case XOR:    bit=XOR_TO_CPU1;  	  break;         
		case IDMA:   bit=IDMA_TO_CPU1;    break;        
		case USB0:   bit=USB0_TO_CPU1;    break;        
		case USB1:   bit=USB1_TO_CPU1;    break;        
		case USB2:   bit=USB2_TO_CPU1;    break;        
		case CESA:   bit=CESA_TO_CPU1;    break;        
		case NOR_FLASH:	bit=NOR_TO_CPU1;    break;        
		case NAND_FLASH: bit=NAND_TO_CPU1;    break; 
		case SPI_FLASH:bit=SPI_TO_CPU1;    break;     
		case TDM: bit=TDM_TO_CPU1;    break;     
		default: bit=0;
			break;
		}
		cpuId = (flag & bit) ? SLAVE_CPU:MASTER_CPU;
		mvSocUnitMapSet(i, cpuId);
	}
	return MV_TRUE;
}

MV_U32 mvSocUnitMapFillFlagFormTable(void)
{
	int i;
	MV_U32 flag = 0;
	for (i = 0; mv_res_table[i].cpuId != -1; i++) 
	{
	    if (mvSocUnitMapGet(i) == SLAVE_CPU)
	    {
		switch (i)
		{
		case UART0:  flag |= UART0_T0_CPU1;   	break;        
		case UART1:  flag |= UART1_TO_CPU1;   	break;       
		case PEX00:  flag |= PEX0_TO_CPU1;    	break;       
		case PEX10:  flag |= PEX1_TO_CPU1;    	break;       
		case GIGA0:  flag |= GIGA0_TO_CPU1;   	break;       
		case GIGA1:  flag |= GIGA1_TO_CPU1;   	break;       
		case GIGA2:  flag |= GIGA2_TO_CPU1;   	break;       
		case GIGA3:  flag |= GIGA3_TO_CPU1;   	break;       
		case SATA:   flag |= SATA_TO_CPU1;    	break;        
		case XOR:    flag |= XOR_TO_CPU1;	break;         
		case IDMA:   flag |= IDMA_TO_CPU1;    	break;        
		case USB0:   flag |= USB0_TO_CPU1;    	break;        
		case USB1:   flag |= USB1_TO_CPU1;    	break;        
		case USB2:   flag |= USB2_TO_CPU1;    	break;        
		case CESA:   flag |= CESA_TO_CPU1;    	break;        
		case NOR_FLASH: flag |= NOR_TO_CPU1;   break;
		case NAND_FLASH: flag |= NAND_TO_CPU1;   break;
		case SPI_FLASH: flag |= SPI_TO_CPU1;   break;
		case TDM: flag |= TDM_TO_CPU1;   break;
		default: 				
			break;
		}
	    }
	}

	return flag;
}

MV_VOID mvSocUnitMapPrint(MV_8* buf)
{
	int i, count;
	count = mvOsSPrintf(buf, "CPU core %d, SoC units in use:\n", whoAmI());
	for (i = 0; mv_res_table[i].cpuId != -1; i++) {
		if (mv_res_table[i].cpuId == whoAmI()) {
			count += mvOsSPrintf(buf+count, "%s ", mv_res_table[i].unitName);
		}
	}
}
