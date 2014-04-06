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

#ifndef __INCETHPHYH
#define __INCETHPHYH

#include "mvCommon.h"
#include "mvOs.h"
#include "mvEthPhyRegs.h"


MV_STATUS	mvEthPhyRegRead(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 *data);
MV_STATUS 	mvEthPhyRegWrite(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 data);
MV_STATUS 	mvEthPhyReset(MV_U32 phyAddr, int timeout);
MV_STATUS 	mvEthPhyRestartAN(MV_U32 phyAddr, int timeout);
MV_STATUS 	mvEthPhyDisableAN(MV_U32 phyAddr, int speed, int duplex);
MV_STATUS   	mvEthPhyLoopback(MV_U32 phyAddr, MV_BOOL isEnable);
MV_BOOL     	mvEthPhyCheckLink(MV_U32 phyAddr);
MV_STATUS	mvEthPhyPrintStatus(MV_U32 phyAddr);
MV_VOID		mvEthE1111PhyBasicInit(MV_U32 ethPortNum);
MV_VOID		mvEthE1112PhyBasicInit(MV_U32 ethPortNum);
MV_VOID		mvEthE1112PhyPowerDown(MV_U32 ethPortNum);
MV_VOID		mvEthE1112PhyPowerUp(MV_U32 ethPortNum);
MV_VOID		mvEthE1116PhyBasicInit(MV_U32 ethPortNum);
MV_VOID		mvEthE1310PhyBasicInit(MV_U32 ethPortNum);
MV_VOID		mvEthE3016PhyBasicInit(MV_U32 ethPortNum);
MV_VOID		mvEthE1011PhyBasicInit(MV_U32 ethPortNum);
MV_VOID 	mvEthSgmiiToCopperPhyBasicInit(MV_U32 ethPortNum);
MV_VOID		mvEth1145PhyBasicInit(MV_U32 ethPortNum);
MV_VOID		mvEth1121PhyBasicInit(MV_U32 ethPortNum);

#if defined(CONFIG_BUFFALO_USE_GPIO_DRIVER)
MV_VOID		mvEthE111xPhyBasicInit_internal(MV_U32 ethPortNum, MV_U32 flag_PolarityChange);
#define mvEthE111xPhyBasicInit(x)			mvEthE111xPhyBasicInit_internal(x, 0)
#define mvEthE111xPhyBasicInitLedPolarityChange(x)      mvEthE111xPhyBasicInit_internal(x, 1)

MV_STATUS	Is_mvEthE111xPhy(MV_U32 ethPortNum);
MV_STATUS	Is_link_mvEthE1111Phy(MV_U32 ethPortNum);
MV_VOID		buffalo_link_led_off(MV_U32 ethPortNum);
MV_VOID		buffalo_link_led_on(MV_U32 ethPortNum);
#endif // defined(CONFIG_BUFFALO_PLATFORM)

#endif /* #ifndef __INCETHPHYH */
