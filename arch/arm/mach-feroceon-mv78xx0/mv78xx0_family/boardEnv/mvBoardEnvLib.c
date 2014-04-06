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

#include "boardEnv/mvBoardEnvLib.h"
#include "boardEnv/mvBoardEnvSpec.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cntmr/mvCntmr.h"
#include "device/mvDevice.h"
#include "gpp/mvGpp.h"
#include "twsi/mvTwsi.h"
#include "cpu/mvCpu.h"
#include "device/mvDeviceRegs.h"
#include "eth-phy/mvEthPhy.h"
#if defined CONFIG_BUFFALO_PLATFORM
#define STRAP_PINS	(MV_GPP0| MV_GPP1| MV_GPP2| MV_GPP3| MV_GPP4| MV_GPP5| MV_GPP6| MV_GPP7)
#include "buffalo/buffalo78100BoardEnv.c"
#endif

/* defines  */
#ifdef DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	

/* defines */
#define REF_TIMER   0   /* reference timer number */

/* local */
MV_BOOL mvBoardIsBootFromNand(MV_VOID);
static MV_DEV_CS_INFO*  boardGetDevEntry(MV_32 devNum, MV_BOARD_DEV_CLASS devClass);

/* Locals */
MV_U32   refClkDevInit(MV_VOID);
MV_VOID  refClkDevRelease(MV_U32 prevState);
MV_U32   refClkDevBitRateGet(MV_VOID);
MV_VOID  refClkDevStart(MV_U32 refClkDevBits);
MV_U32 sysClkRate = -1;
MV_U32 tClkRate = -1;
MV_U32 gBoardId = 0;


MV_DEV_CS_INFO dbBoardCsInfoNor8Boot[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {
#if defined(CONFIG_BUFFALO_PLATFORM)
		 {DEVICE_CS0, N_A, BOARD_DEV_NAND_FLASH, 8},
		 {DEV_BOOCS, N_A, BOARD_DEV_NOR_FLASH, 8},
		 {SPI_CS, N_A, BOARD_DEV_SPI_FLASH, 8},
#else
		 {DEVICE_CS0, N_A, BOARD_DEV_NOR_FLASH, 32},
		 {DEVICE_CS1, N_A, BOARD_DEV_SEVEN_SEG, 4},
		 {DEVICE_CS2, N_A, BOARD_DEV_NAND_FLASH, 8},
		 {DEV_BOOCS, N_A, BOARD_DEV_NOR_FLASH, 8},
#if !defined (MV78XX0_Z0)
		 {SPI_CS, N_A, BOARD_DEV_SPI_FLASH, 8},
#endif
#endif
		 {MAX_TARGETS, 0, 0, 0}
		 };

MV_DEV_CS_INFO dbBoardCsInfoNor32Boot[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {
		/* {DEVICE_CS0, N_A, BOARD_DEV_NOR_FLASH, 8},*/
		 {DEVICE_CS1, N_A, BOARD_DEV_SEVEN_SEG, 4},
		 {DEVICE_CS2, N_A, BOARD_DEV_NAND_FLASH, 8},
		 {DEV_BOOCS, N_A, BOARD_DEV_NOR_FLASH, 32},
#if !defined (MV78XX0_Z0)
		 {SPI_CS, N_A, BOARD_DEV_SPI_FLASH, 8},
#endif
		 {MAX_TARGETS, 0, 0, 0}
		 };


MV_DEV_CS_INFO dbBoardCsBootFromSpiInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {
#if defined(CONFIG_BUFFALO_PLATFORM)
		 {DEVICE_CS0, N_A, BOARD_DEV_NAND_FLASH, 8},
		 {SPI_CS, N_A, BOARD_DEV_SPI_FLASH, 8},
#else
		 {DEVICE_CS0, N_A, BOARD_DEV_NOR_FLASH, 32},
		 {DEVICE_CS1, N_A, BOARD_DEV_SEVEN_SEG, 4},
		 {DEVICE_CS2, N_A, BOARD_DEV_NAND_FLASH, 8},
#if !defined (MV78XX0_Z0)
		 {SPI_CS, N_A, BOARD_DEV_SPI_FLASH, 8},
#endif
#endif
		 {MAX_TARGETS, 0, 0, 0}
		 };

MV_DEV_CS_INFO dbBoard632XCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {
		 {DEV_BOOCS, N_A, BOARD_DEV_NAND_FLASH, 8},
#if !defined (MV78XX0_Z0)
		 {SPI_CS, N_A, BOARD_DEV_SPI_FLASH, 8},
#endif
		 {MAX_TARGETS, 0, 0, 0}
		 };


MV_DEV_CS_INFO db78200BoardCsInfo[] = 
	 /*{deviceCS, params, devType, devWidth}*/			   
	  {
	  {DEVICE_CS1, N_A, BOARD_DEV_SEVEN_SEG, 4},
	  {DEVICE_CS3, N_A, BOARD_DEV_NAND_FLASH, 8},
	  {DEV_BOOCS, N_A, BOARD_DEV_NOR_FLASH, 16},
#if !defined (MV78XX0_Z0)
	  {SPI_CS, N_A, BOARD_DEV_SPI_FLASH, 8},
#endif
	  {MAX_TARGETS, 0, 0, 0}
	  };


MV_DEV_CS_INFO dbBoardCsInfoBootFromNand[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {
	         {DEV_BOOCS, N_A, BOARD_DEV_NAND_FLASH, 8},
#if !defined (MV78XX0_Z0)
		 {SPI_CS, N_A, BOARD_DEV_SPI_FLASH, 8},
#endif
		 {MAX_TARGETS, 0, 0, 0}
		 };	   

MV_DEV_CS_INFO rdPcacBoardCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {
		 {DEV_BOOCS, N_A, BOARD_DEV_NOR_FLASH, 8},
#if !defined (MV78XX0_Z0)
		 {SPI_CS, N_A, BOARD_DEV_SPI_FLASH, 8},
#endif
		 {MAX_TARGETS, 0, 0, 0}
		 };

MV_DEV_CS_INFO rdAmcBoardCsInfo[] = 
		/*{deviceCS, params, devType, devWidth}*/			   
		 {	         
		 {DEV_BOOCS, N_A, BOARD_DEV_NAND_FLASH, 8},
#if !defined (MV78XX0_Z0)
		 {SPI_CS, N_A, BOARD_DEV_SPI_FLASH, 8},
#endif
		 {MAX_TARGETS, 0, 0, 0}
		 };

static MV_DEV_CS_INFO*  boardGetCsArray(MV_VOID)
{
	MV_U32 boardId = mvBoardIdGet();
	if (DB_78200_ID == boardId)
	{
		if (MV_TRUE == mvBoardIsBootFromNand())
			return dbBoardCsInfoBootFromNand;		
		else
			return db78200BoardCsInfo;
	}
	else
	{
		if (RD_78XX0_PCAC_ID == boardId)
			return rdPcacBoardCsInfo;		
		else if (RD_78XX0_AMC_ID == boardId)
			return rdAmcBoardCsInfo;		
		else if (DB_632X_ID == boardId)
			return dbBoard632XCsInfo;	
		else if (MV_TRUE == mvBoardIsBootFromNand())
			return dbBoardCsInfoBootFromNand;		
		else if (MV_TRUE == mvBoardIsBootFromSpi())
			return dbBoardCsBootFromSpiInfo;
		else if (MV_TRUE == mvBoardIsBootFromNor32())
			return dbBoardCsInfoNor32Boot;	
		else 
			return dbBoardCsInfoNor8Boot;
	}
		
}

/*******************************************************************************
* mvBoardEnvInit - Init board
*
* DESCRIPTION:
*		In this function the board environment take care of device bank 
*		initialization.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardEnvInit(MV_VOID)
{
#if defined CONFIG_BUFFALO_PLATFORM
	MV_U32 devNum;
#else
	MV_U32 devNum, gpioLedMask;
#endif
	MV_U32 devBankParam = -1;
	MV_U32 devBankParamWr = -1;
	MV_U32 boardId = mvBoardIdGet();

#if defined CONFIG_BUFFALO_PLATFORM
	buffaloBoardInfoInit();
	mvGppTypeSet(0, 0xffffffff, BUFFALO_DB_78XX0_GPIO_OUTPUT_EN);
	MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), 1 << 30);
	MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), 1 << 31);
#endif 

	if (UNKNOWN_BOARD_ID == boardId)
	{
		mvOsPrintf("mvBoardEnvInit:Board unknown.\n");
		return;
	}

	for (devNum = DEV_CS0; devNum <= BOOT_CS; devNum++)
	{
		switch(devNum)
		{
#if !defined(MV632X)
			case (DEV_CS0):
				if (DB_78200_ID == boardId)
					devBankParam = DEVICE_78200_CS0_PARAM;
				else
					devBankParam = DEVICE_CS0_PARAM;
				devBankParamWr = DEVICE_CS0_PARAM_WR;
				break;
			case (DEV_CS1):
				devBankParam = DEVICE_CS1_PARAM;
				devBankParamWr = DEVICE_CS1_PARAM_WR;
				break;
			case (DEV_CS2):
				if (DB_78200_ID == boardId)
					devBankParam = DEVICE_78200_CS2_PARAM;
				else
					devBankParam = DEVICE_CS2_PARAM;
				devBankParamWr = DEVICE_CS2_PARAM_WR;
				break;
			case (DEV_CS3):
				if (DB_78200_ID == boardId)
					devBankParam = DEVICE_78200_CS3_PARAM;
				else
					devBankParam = DEVICE_CS3_PARAM;
				devBankParamWr = DEVICE_CS3_PARAM_WR;
				break;
#endif
			case (BOOT_CS):
				if (DB_78200_ID == boardId)
					devBankParam = BOOTDEV_78200_CS_PARAM;
				else
					devBankParam = MV_REG_READ(DEV_BANK_PARAM_REG(devNum));
				devBankParam &= DBP_DEVWIDTH_MASK;
				devBankParam |= BOOTDEV_CS_PARAM;
				devBankParamWr = DEVICE_CS3_PARAM_WR;
				break;
			default:
				devBankParam = -1;
				devBankParamWr = -1;
				break;
	   	}	    
        
		if(devBankParam != -1)
		    MV_REG_WRITE(DEV_BANK_PARAM_REG(devNum), devBankParam);
		if(devBankParamWr != -1)
		    MV_REG_WRITE(DEV_BANK_PARAM_REG_WR(devNum), devBankParamWr);

		devBankParam = -1;
		devBankParamWr = -1;
	}

    	/* Check if boot from NAND - do nothing */
	if (!mvBoardIsBootFromNand())
	{
	    /* Assign NAND CS and care in control reg */
	    if (DB_78200_ID == boardId)
		    mvDevNandDevCsSet(MV_NAND_78200_CS, MV_NAND_CARE);
	    else if (DB_632X_ID == boardId)
            mvDevNandDevCsSet(MV_NAND_632X_CS, MV_NAND_CARE);
        else if (RD_78XX0_AMC_ID == boardId)
		    mvDevNandDevCsSet(MV_NAND_AMC_CS, MV_NAND_CARE);
	    else
            mvDevNandDevCsSet(MV_NAND_CS, MV_NAND_CARE);
	}	

#if defined(CONFIG_BUFFALO_PLATFORM)
	mvDevNandDevCsSet(DEV_CS0, MV_NAND_CARE);
#endif

#ifndef CONFIG_BUFFALO_PLATFORM
    /* Set GPP value before set OE to prevent reset */
     switch (boardId) {
	case DB_78XX0_ID:
		MV_REG_WRITE(GPP_DATA_OUT_SET_REG, (1 << DB_78XX0_SW_RESET));
	   	break;
	case DB_78200_ID:	
		MV_REG_WRITE(GPP_DATA_OUT_CLEAR_REG, (1 << DB_78200_SW_RESET));    
		break;
	case RD_78XX0_AMC_ID:
		MV_REG_WRITE(GPP_DATA_OUT_CLEAR_REG, (1 << RD_AMC_SW_RESET));    
		break;
	case RD_78XX0_MASA_ID:
		MV_REG_WRITE(GPP_DATA_OUT_SET_REG, (1 << RD_MASA_DEBUG_LED_GPP_PIN(1)));
		break;
	case RD_78XX0_H3C_ID:
		MV_REG_WRITE(GPP_DATA_OUT_SET_REG, (1 << RD_H3C_SW_RESET_SELECT)); 
		break;
	 case DB_632X_ID:
         MV_REG_WRITE(GPP_DATA_OUT_SET_REG, (1 << DB_6323_SW_RESET)); 
         /* Change SATA active led polarity for DB-6323 */
         MV_REG_BIT_SET(0xa002c, BIT3);
         break;
	case BUFFALO_DB_78XX0_ID:
		gpioLedMask |= BUFFALO_DB_78XX0_GPIO_OUTPUT_EN;
		break;
	default:
		break;
	}
    /* Debug Led operates via GPP. Initialize GPP */
    gpioLedMask = mvBoardDbgLedGpioMaskGet();
    switch(boardId) {
	case DB_78XX0_ID:
		gpioLedMask |= 1 << DB_78XX0_SW_RESET;
#ifdef MV_INCLUDE_MODEM_ON_TTYS1
		gpioLedMask |= 1 << DB_78XX0_SERIAL_DTR;  /* set DTR as output*/ 
#endif
		break;
	case DB_78200_ID:	
		   gpioLedMask |= DB_78200_GPIO_OUTPUT_EN;
		   break;
	case RD_78XX0_AMC_ID:
		   gpioLedMask |= RD_AMC_GPIO_OUTPUT_EN;
		   break;
	case RD_78XX0_MASA_ID:
		   gpioLedMask |= RD_MASA_GPIO_OUTPUT_EN;  /* set DTR as output*/ 
		   break;
	case RD_78XX0_H3C_ID:
		   gpioLedMask |= RD_H3C_GPIO_OUTPUT_EN;  /* set DTR as output*/ 
		   break;
	case DB_632X_ID:
        gpioLedMask |= DB_6323_GPIO_OUTPUT_EN;
        break;
	case BUFFALO_DB_78XX0_ID:
	default:
    		break;
    }

    /* Set the GPP interrupt pins to output.                */
    mvGppTypeSet  (0, gpioLedMask, (gpioLedMask & MV_GPP_OUT));	    
#endif
}

/*******************************************************************************
* mvBoardIsBootFromNand - 
*
* DESCRIPTION:
*       This routine returns MV_TRUE if the board is configure to boot from nand
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - boot from nand.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsBootFromNand(MV_VOID)
{
    /* Check if boot from NAND - do nothing */
    if (((MV_REG_READ(CPU_RESET_SAMPLE_L_REG) & MSAR_BOOTDEV_MASK) == MSAR_BOOTDEV_DCE_NAND) ||
	    ((MV_REG_READ(CPU_RESET_SAMPLE_L_REG) & MSAR_BOOTDEV_MASK) == MSAR_BOOTDEV_CE_NAND))
	{
		return MV_TRUE;
	}
	return MV_FALSE;
} 
/*******************************************************************************
* mvBoardIsBootFromSpi - 
*
* DESCRIPTION:
*       This routine returns MV_TRUE if the board is configure to boot from SPI flash
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - boot from SPI.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsBootFromSpi(MV_VOID)
{
    /* Check if boot from NAND - do nothing */
    if ((MV_REG_READ(CPU_RESET_SAMPLE_L_REG) & MSAR_BOOTDEV_MASK) == MSAR_BOOTDEV_SPI)
	{
	return MV_TRUE;
	}
	return MV_FALSE;
} 
/*******************************************************************************
* mvBoardIsBootFromNor - 
*
* DESCRIPTION:
*       This routine returns MV_TRUE if the board is configure to boot from NOR flash
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - boot from Nor.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsBootFromNor(MV_VOID)
{
    /* Check if boot from NAND - do nothing */
    if ((MV_REG_READ(CPU_RESET_SAMPLE_L_REG) & MSAR_BOOTDEV_MASK) == MSAR_BOOTDEV_FLASH)
	{
		return MV_TRUE;
	}
	return MV_FALSE;
} 

/*******************************************************************************
* mvBoardIsBootFromNor32 - 
*
* DESCRIPTION:
*       This routine returns MV_TRUE if the board is configure to boot from NOR 32-bit 
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - boot from Nor.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsBootFromNor32(MV_VOID)
{
    /* Check if boot from NAND - do nothing */
    if ((MV_REG_READ(CPU_RESET_SAMPLE_L_REG) & MSAR_BOOTDEV_WIDTH_MASK) 
		== MSAR_BOOTDEV_WIDTH_32BIT)
    {
		return MV_TRUE;
    }
    return MV_FALSE;
} 

/*******************************************************************************
* mvBoardNameGet - Get Board name
*
* DESCRIPTION:
*       This function returns a string describing the board model and revision.
*       String is extracted from board I2C EEPROM.
*
* INPUT:
*       None.
*
* OUTPUT:
*       pNameBuff - Buffer to contain board name string. Minimum size 32 chars.
*
* RETURN:
*       
*       MV_ERROR if informantion can not be read.
*******************************************************************************/
MV_STATUS mvBoardNameGet(char *pNameBuff)
{
	char ctrlStr[132] = "";
	MV_U32 boardId = mvBoardIdGet();
	mvCtrlNameGet(ctrlStr);
	
	if (RD_78XX0_H3C_ID == boardId)
	{
		sprintf (pNameBuff, "RD-");
		strcat  (pNameBuff, ctrlStr);
		strcat  (pNameBuff, "-H3C");
	}
	else
	if (DB_78XX0_ID == boardId || 
	    DB_78200_ID == boardId || 
	    DB_632X_ID == boardId)
	{
		sprintf (pNameBuff, "DB-");
		strcat  (pNameBuff, ctrlStr);
		strcat  (pNameBuff, "-A-BP");
	}
	else
	if (RD_78XX0_AMC_ID == boardId)
	{
		sprintf (pNameBuff, "DB-");
		strcat  (pNameBuff, ctrlStr);
		strcat  (pNameBuff, "-A-AMC");
	}
	else
	if (RD_78XX0_MASA_ID == boardId)
	{
		sprintf (pNameBuff, "RD-");
		strcat  (pNameBuff, ctrlStr);
		strcat  (pNameBuff, "-MASA");
	}
	else
	if (RD_78XX0_PCAC_ID == boardId)
	{
		sprintf (pNameBuff, "RD-");
		strcat  (pNameBuff, ctrlStr);
		strcat  (pNameBuff, "-PCAC");
	}
	else
	if (BUFFALO_DB_78XX0_ID == boardId)
	{
		sprintf (pNameBuff, "DB-");
		strcat  (pNameBuff, ctrlStr);
		strcat  (pNameBuff, "-A-BP-BUFFALO");
	}
	else
	{
		sprintf (pNameBuff, "Board unknown.\n");
		return MV_ERROR;
	}

	return MV_OK;
}

/*******************************************************************************
* mvBoardIsPortInSgmii - 
*
* DESCRIPTION:
*       This routine returns MV_TRUE for port number works in SGMII or MV_FALSE
*	For all other options.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE - port in SGMII.
*       MV_FALSE - other.
*
*******************************************************************************/
MV_BOOL mvBoardIsPortInSgmii(MV_U32 ethPortNum)
{
    MV_BOOL ethPortSgmiiSupport[BOARD_ETH_PORT_NUM] = MV_ETH_PORT_SGMII;

    if (ethPortNum > BOARD_ETH_END_PORT_NUM)
    {
	    mvOsPrintf ("mvBoardIsPortInSgmii: Invalid portNo=%d.\n", ethPortNum);
		return MV_FALSE;
	}
    return ethPortSgmiiSupport[ethPortNum - BOARD_ETH_START_PORT_NUM];
} 

/*******************************************************************************
* mvBoardPhyAddrGet - Get the phy address
*
* DESCRIPTION:
*       This routine returns the Phy address of a given ethernet port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit describing Phy address, '-1' if the port number is wrong.
*
*******************************************************************************/
MV_U32 mvBoardPhyAddrGet(MV_U32 ethPortNum)
{
    MV_U32  ethPortPhyAddr[BOARD_ETH_PORT_NUM] = MV_ETH_PORT_PHY_ADDR;

    if(ethPortNum > BOARD_ETH_END_PORT_NUM)
    {
	    mvOsPrintf ("mvBoardPhyAddrGet: Invalid portNo=%d.\n", ethPortNum);
		return MV_ERROR;
	}

    return ethPortPhyAddr[ethPortNum - BOARD_ETH_START_PORT_NUM];
}


/*******************************************************************************
* mvBoardMacSpeedGet - Get the Mac speed
*
* DESCRIPTION:
*       This routine returns the Mac speed if pre define of a given ethernet port.
*
* INPUT:
*       ethPortNum - Ethernet port number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_BOARD_MAC_SPEED, -1 the port number is wrong.
*
*******************************************************************************/
MV_BOARD_MAC_SPEED      mvBoardMacSpeedGet(MV_U32 ethPortNum)
{
    MV_32 boardId = mvBoardIdGet();
    if (boardId == RD_78XX0_H3C_ID)
	/* RD-H3C 1145 works in RGMII to SGMII mode which disable speed AN,
	 force speed to 1000*/
	return BOARD_MAC_SPEED_1000M;
    
    if ((boardId == RD_78XX0_AMC_ID) && (ethPortNum == 3))
	/* RD-AMC 1145 port 3 works in RGMII to SGMII mode which disable speed AN,
	force speed to 100 */
	return BOARD_MAC_SPEED_100M;

    return BOARD_MAC_SPEED_AUTO;
}
/*******************************************************************************
* mvBoardSpecInitGet -
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN: Return MV_TRUE and parameters in case board need specific phy init, 
* 	  otherwise return MV_FALSE. 
*
*
*******************************************************************************/

MV_BOOL mvBoardSpecInitGet(MV_U32* regOff, MV_U32* data)
{
	return MV_FALSE;
}

/*******************************************************************************
* mvBoardTclkGet - Get the board Tclk (Controller clock)
*
* DESCRIPTION:
*       This routine extract the controller T clock.
*
* INPUT:
*	None.
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_32 mvBoardTclkGet(MV_VOID)
{
	MV_32 tmpTClkRate=0;

	if (-1 == tClkRate)
	{
		tmpTClkRate = MV_REG_READ(CPU_RESET_SAMPLE_H_REG);
		/* Internal PLL */
		if ((tmpTClkRate & MSAR_TCLCK_MODE_MASK) == MSAR_TCLCK_MODE_PLL)
		{
			switch(tmpTClkRate & MSAR_TCLCK_MASK)
			{
			case(MSAR_TCLCK_167):
				tmpTClkRate = MV_BOARD_TCLK_166MHZ;
				break;
			case(MSAR_TCLCK_200):
				tmpTClkRate = MV_BOARD_TCLK_200MHZ;
				break;
			default:
				tmpTClkRate = MV_BOARD_TCLK_200MHZ;
				break;
			}
		}
		else
		/* External PLL */
		{
			switch(tmpTClkRate & MSAR_TCLCK_DES_MASK)
			{
			case(MSAR_TCLCK_DES_167):
				tmpTClkRate = MV_BOARD_TCLK_166MHZ;
				break;
			case(MSAR_TCLCK_DES_200):
				tmpTClkRate = MV_BOARD_TCLK_200MHZ;
				break;
			default:
				tmpTClkRate = MV_BOARD_TCLK_200MHZ;
				break;
			}
		}

		tClkRate = tmpTClkRate ;
	}
	else
	{
		tmpTClkRate = tClkRate;
	}


	return tmpTClkRate;
}

/*******************************************************************************
* mvBoardSysClkGet - Get the board SysClk (CPU bus clock)
*
* DESCRIPTION:
*       This routine extract the CPU bus clock.
*		Note: In the MPC745x, DEC is decremented and the time base 
*		increments at 1/4 of the system bus clock frequancy.
*		Note: In order to avoid interference, make sure task context switch
*		and interrupts will not occure during this function operation
*
* INPUT:
*       countNum - Counter number.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit clock cycles in Hertz.
*
*******************************************************************************/
MV_32  mvBoardSysClkGet(MV_VOID)
{
	MV_32 tmpSysClkRate=0;
	
	if (-1 == sysClkRate)
	{	
		tmpSysClkRate = MV_REG_READ(CPU_RESET_SAMPLE_L_REG);
		switch(tmpSysClkRate & MSAR_SYSCLCK_MASK)
		{
		case(MSAR_SYSCLCK_200):
			tmpSysClkRate = MV_BOARD_SYSCLK_200MHZ;
			break;
		case(MSAR_SYSCLCK_267):
			tmpSysClkRate = MV_BOARD_SYSCLK_267MHZ;
			break;
		case(MSAR_SYSCLCK_333):
			tmpSysClkRate = MV_BOARD_SYSCLK_333MHZ;
			break;
		case(MSAR_SYSCLCK_400):
			tmpSysClkRate = MV_BOARD_SYSCLK_400MHZ;
			break;
		case(MSAR_SYSCLCK_250):
			tmpSysClkRate = MV_BOARD_SYSCLK_250MHZ;
			break;
		case(MSAR_SYSCLCK_300):
			tmpSysClkRate = MV_BOARD_SYSCLK_300MHZ;
			break;
		default:
			tmpSysClkRate = MV_BOARD_SYSCLK_267MHZ;
			break;
		}

		sysClkRate = tmpSysClkRate;
	}
	else
	{
		tmpSysClkRate = sysClkRate;
	}

		return tmpSysClkRate;
}

/*******************************************************************************
* mvBoardDebugLed - Set the board debug LEDs
*
* DESCRIPTION:
*       This function sets the board debug LEDs as the input number in hex,
*       e.g - if state == 3 -->> debug leds 0 and 1 are turned on
*
* INPUT:
*       hexNum - Number to be displied in hex by LEDs.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardDebugLed(MV_U32 binNum)
{
	int ledNum;
	MV_32 boardId = mvBoardIdGet();

	if (BUFFALO_DB_78XX0_ID == boardId)
		return;

	/* Binary number limited by the number of LEDs */
	binNum %= (1 << MV_BOARD_LED_NUM);

	for(ledNum = 0 ; ledNum < MV_BOARD_LED_NUM ; ledNum++)
	{
	    if(binNum & (1 << ledNum)) /* turn led on */
	    {
		if (DB_78XX0_ID == boardId || DB_632X_ID == boardId)
			mvGppValueSet( 0, 
					   (1 << DB_78XX0_LED_GPP_PIN(ledNum)),  /* mask */
					   (1 << DB_78XX0_LED_GPP_PIN(ledNum))); /* value */
		else if (RD_78XX0_AMC_ID == boardId)
			mvGppValueSet( 0, 
				       (1 << RD_AMC_LED_GPP_PIN(ledNum)),  /* mask */
				       (1 << RD_AMC_LED_GPP_PIN(ledNum))); /* value */
	    }      
	    else /* turn led off */
	    {
		if (DB_78XX0_ID == boardId || DB_632X_ID == boardId)
			mvGppValueSet( 0, 
						   (1 << DB_78XX0_LED_GPP_PIN(ledNum)),  /* mask */
						  ~(1 << DB_78XX0_LED_GPP_PIN(ledNum))); /* value */
		if (RD_78XX0_AMC_ID == boardId)
			mvGppValueSet( 0, 
						   (1 << RD_AMC_LED_GPP_PIN(ledNum)),  /* mask */
						  ~(1 << RD_AMC_LED_GPP_PIN(ledNum))); /* value */
	    }
	}
}


/*******************************************************************************
* mvBoardDebug7Seg - Set the board debug 7Seg
*
* DESCRIPTION:
*
* INPUT:
*       hexNum - Number to be displied in hex by 7Seg.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvBoardDebug7Seg(MV_U32 hexNum)
{
    MV_U32 	dummy, address;
	
    MV_32 boardId = mvBoardIdGet();

    if (BUFFALO_DB_78XX0_ID == boardId)
	return;

    if (DB_632X_ID == boardId)
    {
        MV_REG_WRITE(GPP_DATA_OUT_REG(0), (MV_REG_READ(GPP_DATA_OUT_REG(0))& ~(0xf << 17)) | (hexNum << 17));
    }
    else if ((DB_78XX0_ID == boardId) ||  (DB_78200_ID == boardId))
#if !defined (MV78XX0_Z0)
        MV_MEMIO8_WRITE(MV_BOARD_7SEG_BASE, hexNum);
#else
    {
    
	    address = MV_BOARD_7SEG_BASE + (hexNum << 4);
	    dummy = MV_MEMIO32_READ(CPU_MEMIO_UNCACHED_ADDR(address));
    }
#endif
    else
    {
	/* The 7Seg on the device bus uses the address to set the sement */
	if (mvBoardIdGet() == RD_78XX0_H3C_ID) {	
		dummy = (hexNum % 4);
		MV_MEMIO8_WRITE(CPU_MEMIO_UNCACHED_ADDR(MV_BOARD_7SEG_BASE), dummy);
	}
#if !defined (MV78XX0_Z0)
	else {
		address = MV_BOARD_7SEG_BASE + (hexNum << 4);
		dummy = MV_MEMIO32_READ(CPU_MEMIO_UNCACHED_ADDR(address));
	}
#endif
    }
}
/*******************************************************************************
* mvBoardReset - mvBoardReset
*
* DESCRIPTION:
*			Reset the board
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*       None
*
*******************************************************************************/
MV_VOID	mvBoardReset(MV_VOID)
{
	MV_32 boardId = mvBoardIdGet();
	if (boardId == BUFFALO_DB_78XX0_ID)
	{
		MV_REG_WRITE(GPP_DATA_OUT_REG(0), (1 << BUFFALO_DB_78XX0_SW_RESET));
		while(1);
	}

	if ((boardId == DB_78XX0_ID) ||
	    (boardId == RD_78XX0_MASA_ID) || 
        (boardId == DB_632X_ID) ||
	    (boardId == RD_78XX0_H3C_ID))
	{
		MV_REG_WRITE(CPU_RESET_OUT_MASK_REG, (CPU_SOFT_RESET_OUT_MASK));
		MV_REG_WRITE(CPU_SOFT_RESET_OUT_REG, (CPU_SOFT_RESET_OUT));
		while(1);
	}
	
	if (boardId == DB_78200_ID)
	{
		MV_REG_WRITE(GPP_DATA_OUT_REG(0), (1 << DB_78200_SW_RESET));
		while(1);
	}
	
	if (boardId == RD_78XX0_AMC_ID)
	{
		MV_REG_BIT_SET(GPP_DATA_OUT_REG(0), (1 << RD_AMC_SW_RESET ));
		MV_REG_BIT_RESET(GPP_DATA_OUT_EN_REG(0), (1 << RD_AMC_SW_RESET ));
		while(1);
	}
}

/*******************************************************************************
* mvBoardUSBVbusGpioPinGet - Get board GPP pin number connected to USB VBUS
*
* DESCRIPTION:
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardUSBVbusGpioPinGet(int devId)
{
	MV_U32 boardID;
	
	boardID = mvBoardIdGet();
	
	if (boardID == DB_78XX0_ID || 
	    boardID == DB_78200_ID || 
	    boardID == DB_632X_ID)
	{
		return DB_78XX0_USB_VBUS_PIN(devId);
	}
	
	if (boardID == RD_78XX0_AMC_ID)
	{
		return RD_AMC_USB_VBUS_PIN(devId);
	}
	
	if (boardID == RD_78XX0_H3C_ID)
	{
		return RD_H3C_USB_VBUS_PIN(devId);
	}
	
	return MV_ERROR;
}

/*******************************************************************************
* mvBoardUSBVbusEnGpioPinGet - return Vbus Enable output GPP
*
* DESCRIPTION:
*
* INPUT:
*		int  devNo.
*
* OUTPUT:
*		None.
*
* RETURN:
*       GPIO pin number. The function return -1 for bad parameters.
*
*******************************************************************************/
MV_32 mvBoardUSBVbusEnGpioPinGet(int devId)
{
	return N_A;
}
/*******************************************************************************
* mvBoardGpioIntMaskGet - Get GPIO mask for interrupt pins
*
* DESCRIPTION:
*		This function returns a 32-bit mask of GPP pins that connected to 
*		interrupt generating sources on board.
*		For example if UART channel A is hardwired to GPP pin 8 and 
*		UART channel B is hardwired to GPP pin 4 the fuinction will return
*		the value 0x000000110
*
* INPUT:
*       None.
*
* OUTPUT:
*		None.
*
* RETURN:
*		See description. The function return -1 if board is not identified.
*
*******************************************************************************/
MV_32 mvBoardGpioIntMaskGet(MV_VOID)
{	
	MV_U32 boardID;

	boardID = mvBoardIdGet();

	if (boardID == RD_78XX0_MASA_ID)
	{
	    return RD_78XX0_MASA_INT_MASK;
	}

	return MV_ERROR;  /* Error */
}
/*******************************************************************************
* mvBoardMppGet - Get board dependent MPP register value
*
* DESCRIPTION:
*		MPP settings are derived from board design.
*		MPP group consist of 8 MPPs. An MPP group represent MPP 
*		control register.
*       This function retrieves board dependend MPP register value.
*
* INPUT:
*       mppGroupNum - MPP group number. 
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit value describing MPP control register value. 
*
*******************************************************************************/
MV_U32 mvBoardMppGet(MV_U32 mppGroupNum)
{
	MV_U32 boardID;
    
	boardID = mvBoardIdGet();

	/* This id DB-MV78100-BP */
	if (DB_78XX0_ID == boardID)
	{
		switch(mppGroupNum)
		{
			case (0):
				return DB_78XX0_MPP0_7;
			case (1):
				return DB_78XX0_MPP8_15;
			case (2):
				if (mvBoardIsBootFromNand())
					return DB_78XX0_MPP16_23_NAND;
				else
					return DB_78XX0_MPP16_23;
			case (3):
				return DB_78XX0_MPP24_31;
			default:
				return 0;
		}
	}
	else if(BUFFALO_DB_78XX0_ID == boardID)
	{
		switch(mppGroupNum)
		{
			case (0):
				return BUFFALO_DB_78XX0_MPP0_7;
			case (1):
				return BUFFALO_DB_78XX0_MPP8_15;
			case (2):
				return BUFFALO_DB_78XX0_MPP16_23;
			case (3):
				return BUFFALO_DB_78XX0_MPP24_31;
			case (4):
				return BUFFALO_DB_78XX0_MPP32_39;
			case (5):
				return BUFFALO_DB_78XX0_MPP40_47;
			case (6):
				return BUFFALO_DB_78XX0_MPP48_55;
			default:
				return 0;
		}
	}
	/* This id DB-MV78200-BP */
	else if (DB_78200_ID == boardID)
	{
		switch(mppGroupNum)
		{
			case (0):
				return DB_78200_MPP0_7;
			case (1):
				return DB_78200_MPP8_15;
			case (2):
				return DB_78200_MPP16_23;
			case (3):
				return DB_78200_MPP24_31;
			case (4):
				return DB_78200_MPP32_39;
			default:
				return 0;
		}
	}
	/* This id DB-MV88F632X-BP */
	else if (DB_632X_ID == boardID)
	{	
		switch(mppGroupNum)
		{
			case (0):
				return DB_632X_MPP0_7;
			case (1):
				return DB_632X_MPP8_15;
			case (2):
				return DB_632X_MPP16_23;
			case (3):
                return DB_632X_MPP32_39;
            case (4):
                return DB_632X_MPP40_47;
            case (5):
				return DB_632X_MPP24_31;
			default:
				return 0;
		}
	}
	else if(RD_78XX0_AMC_ID == boardID)
	{
		switch(mppGroupNum)
		{
			case (0):
				return RD_AMC_MPP0_7;
			case (1):
				return RD_AMC_MPP8_15;
			case (2):
				return RD_AMC_MPP16_23;
			case (3):
				return RD_AMC_MPP24_31;
			case (4):
				return RD_AMC_MPP32_39;
			case (5):
				return RD_AMC_MPP40_47;
			case (6):
				return RD_AMC_MPP48_49;
			default:
                DB(mvOsPrintf("mvBoardMppGet: ERR. Unsupported MPP group %d\n",
							  mppGroupNum));
		}
	}
	else if(RD_78XX0_H3C_ID == boardID)
	{
		switch(mppGroupNum)
		{
			case (0):
				return RD_H3C_MPP0_7;
			case (1):
				return RD_H3C_MPP8_15;
			case (2):
				return RD_H3C_MPP16_23;
			case (3):
				return RD_H3C_MPP24_31;
			default:
                DB(mvOsPrintf("mvBoardMppGet: ERR. Unsupported MPP group %d\n",
							  mppGroupNum));
		}
	}
	else if(RD_78XX0_MASA_ID == boardID)
	{
		switch(mppGroupNum)
		{
			case (0):
				return RD_MASA_MPP0_7;
			case (1):
				return RD_MASA_MPP8_15;
			case (2):
				return RD_MASA_MPP16_23;
			case (3):
				return RD_MASA_MPP24_31;
			default:
				DB(mvOsPrintf("mvBoardMppGet: ERR. Unsupported MPP group %d\n",
							  mppGroupNum));
		}
	}
	else
	{
		DB(mvOsPrintf("mvBoardMppGet: ERR. Board not spported\n"));
	}
	
	return 0;
}
/* Board devices API managments */

/*******************************************************************************
* mvBoardGetDeviceNumber - Get number of device of some type on the board
*
* DESCRIPTION:
*
* INPUT:
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		number of those devices else the function returns 0
*
*
*******************************************************************************/
MV_32 mvBoardGetDevicesNumber(MV_BOARD_DEV_CLASS devClass)
{
	MV_32 i, count = 0;
	MV_DEV_CS_INFO* devEntry = boardGetCsArray();

	for (i = 0; devEntry[i].deviceCS != MAX_TARGETS; i++)
	{
		if (devClass == devEntry[i].devClass)
			count++;
	}
	return count;
}

/*******************************************************************************
* mvBoardGetDeviceBaseAddr - Get base address of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		Base address else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_U32 mvBoardGetDeviceBaseAddr(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO* devEntry = boardGetDevEntry(devNum,devClass);
	if (devEntry != NULL)
	{
		return mvCpuIfTargetWinBaseLowGet(devEntry->deviceCS);
	}
	return 0xFFFFFFFF;
}


/*******************************************************************************
* mvBoardGetDeviceBusWidth - Get Bus width of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		Bus width else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_U32 mvBoardGetDeviceBusWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO* devEntry = boardGetDevEntry(devNum,devClass);
	if (devEntry != NULL)
	{
		MV_DEVICE dev;
		MV_U32 ret;
		/*Convert target to device*/
		switch(devEntry->deviceCS)
		{	
			case DEVICE_CS0:
				dev = DEV_CS0;
				break;
			case DEVICE_CS1:
				dev = DEV_CS1;
				break;
			case DEVICE_CS2:
				dev = DEV_CS2;
				break;
			case DEVICE_CS3:
				dev = DEV_CS3;
				break;
			case DEV_BOOCS:
				dev = BOOT_CS;
				break;
			default:
				return 0xFFFFFFFF;
		}
		ret = mvDevWidthGet(dev);
		return ret;
	}

	return 0xFFFFFFFF;
}


/*******************************************************************************
* mvBoardGetDeviceWinSize - Get the window size of a device existing on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		window size else the function returns 0xffffffff
*
*
*******************************************************************************/
MV_U32 mvBoardGetDeviceWinSize(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO* devEntry = boardGetDevEntry(devNum,devClass);
	if (devEntry != NULL)
	{
		return mvCpuIfTargetWinSizeGet(devEntry->deviceCS);
	}
	return 0xFFFFFFFF;
}



/*******************************************************************************
* mvBoardRtcTwsiAddrTypeGet - 
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*
*
*******************************************************************************/
MV_U8 mvBoardRtcTwsiAddrTypeGet()
{
	return MV_BOARD_RTC_I2C_ADDR_TYPE;
}
		
/*******************************************************************************
* mvBoardRtcTwsiAddrGet - 
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*
*
*******************************************************************************/
MV_U8 mvBoardRtcTwsiAddrGet()
{
	return MV_BOARD_RTC_I2C_ADDR;
}


/*******************************************************************************
* boardGetDevEntry - returns the entry pointer of a device on the board
*
* DESCRIPTION:
*
* INPUT:
*       devIndex - The device sequential number on the board
*		devType - The device type ( Flash,RTC , etc .. )
*
* OUTPUT:
*       None.
*
* RETURN:
*       If the device is found on the board the then the functions returns the
*		dev number else the function returns 0x0
*
*
*******************************************************************************/
static MV_DEV_CS_INFO*  boardGetDevEntry(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_U32	foundIndex = 0, i;
	MV_DEV_CS_INFO* devEntry = boardGetCsArray();

	for (i = 0; devEntry[i].deviceCS != MAX_TARGETS; i++)	
	{
		if (devEntry[i].devClass == devClass)
		{
            		if (foundIndex == devNum)
			{
				return &devEntry[i];
			}
			foundIndex++;
		}
	}

	/* device not found */
	return NULL;
}


/*******************************************************************************
* mvBoardGetDevCS - 
*
* DESCRIPTION:
*		Return device CS target
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*       None
*
*******************************************************************************/
MV_TARGET mvBoardGetDevCSNum(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO* devEntry = boardGetDevEntry(devNum,devClass);
	if (devEntry != NULL)
	{
		return devEntry->deviceCS;
	}
	return MAX_TARGETS;	
}



MV_32 mvBoardGetDeviceWidth(MV_32 devNum, MV_BOARD_DEV_CLASS devClass)
{
	MV_DEV_CS_INFO* devEntry = boardGetDevEntry(devNum,devClass);
	if (devEntry != NULL)
		return devEntry->devWidth;

	return 0xFFFFFFFF;

}


MV_U32 mvBoardFlashParamGet(MV_VOID)
{
	MV_U32 boardId = mvBoardIdGet();
	if (RD_78XX0_AMC_ID == boardId  || 
		RD_78XX0_MASA_ID == boardId || 
		 RD_78XX0_H3C_ID == boardId ||
		DB_632X_ID == boardId)
		return 0;
#if defined CONFIG_BUFFALO_PLATFORM
	else if (BUFFALO_DB_78XX0_ID == boardId)
		/* Buffalo-product's main flush is NAND. */
		return MV_BOARD_NFLASH_PARAM;
#endif
	else
		return MV_BOARD_FLASH_PARAM_VAL;
}

MV_U32 mvBoardGetMaxUsbIf(MV_VOID)
{
	MV_U32 boardId = mvBoardIdGet();
	if (RD_78XX0_AMC_ID == boardId  || 		
		 RD_78XX0_H3C_ID == boardId)
		return 1;
	else
		return 3;
}


/*********************************************************** 
* Init the PHY or Switch of the board 			   *
 ***********************************************************/
MV_VOID mvBoardEgigaPhySwitchInit(void) 
{
    MV_U16 port;
    MV_U32 boardID = mvBoardIdGet();


	/* First init the SGMII to copper on the test board */
	if (boardID == RD_78XX0_H3C_ID)
	    for (port = BOARD_ETH_START_PORT_NUM;port <= BOARD_ETH_END_PORT_NUM; port++)
	    {
		mvEthSgmiiToCopperPhyBasicInit(port);
		mvEth1145PhyBasicInit(port);
	    }

	/* This id RD-78XX0-AMC */
	if (boardID == RD_78XX0_AMC_ID)
	    for (port = BOARD_ETH_START_PORT_NUM;port <= BOARD_ETH_END_PORT_NUM; port++)
	    {
		    mvEth1145PhyBasicInit(port);
	    }
	else if (boardID == BUFFALO_DB_78XX0_ID)
//			mvEthE111xPhyBasicInit(port);

	/* This id DB-78XX0-BP */
	if ((boardID == DB_78XX0_ID) || (boardID == RD_78XX0_MASA_ID)  || (boardID == DB_632X_ID))
	    for (port = BOARD_ETH_START_PORT_NUM;port <= BOARD_ETH_END_PORT_NUM; port++)
		mvEth1121PhyBasicInit(port);

	/* This id DB-78200-BP */
	if (boardID == DB_78200_ID)
	    for (port = BOARD_ETH_START_PORT_NUM;port <= BOARD_ETH_END_PORT_NUM; port++)
	    {	
		if ((port == 0) || (port == 1))
		    mvEth1121PhyBasicInit(port);
		else
		    mvEthE1116PhyBasicInit(port);
	    }

	/* This id RD_MV78XX0_PCAC */
	if (boardID == RD_78XX0_PCAC_ID)
		mvEthE1116PhyBasicInit(0);
}

/*******************************************************************************
* mvBoardIdGet - Get Board model
*
* DESCRIPTION:
*       This function returns board ID.
*       Board ID is 32bit word constructed of board model (16bit) and 
*       board revision (16bit) in the following way: 0xMMMMRRRR.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit board ID number, '-1' if board is undefined.
*
*******************************************************************************/
MV_U32 mvBoardIdGet(MV_VOID)
{
	MV_U32 ctrlModel;
	if (gBoardId != 0)
		return gBoardId;
/*No detection mechanism*/
#if defined(CONFIG_BUFFALO_PLATFORM)
	return BUFFALO_DB_78XX0_ID;
#elif defined(RD_MV78XX0_AMC)
	return RD_78XX0_AMC_ID;
#elif defined(RD_MV78XX0_MASA)
	return RD_78XX0_MASA_ID;
#elif defined(RD_MV78XX0_H3C)
	return RD_78XX0_H3C_ID;
#elif defined(RD_MV78XX0_PCAC)
	return RD_78XX0_PCAC_ID;
#endif

	ctrlModel = mvCtrlModelGet();

	if (MV_78XX0_DEV_ID == ctrlModel || 
	    MV_78100_DEV_ID == ctrlModel || 
	    MV_76100_DEV_ID == ctrlModel)
	{
		return DB_78XX0_ID;
	}
	else
	if (MV_78200_DEV_ID == ctrlModel)
	{
		return DB_78200_ID;
	}
	else
	if (MV_6321_DEV_ID == ctrlModel ||
	    MV_6322_DEV_ID == ctrlModel ||
	    MV_6323_DEV_ID == ctrlModel)
	{
		return DB_632X_ID;
	}	
	else
		return UNKNOWN_BOARD_ID;
}
/*******************************************************************************
* refClkDevInit - Initialize the reference clock device.
*
* DESCRIPTION:
*		This function initialize the on board DUART device to be used as 
*		reference clock device which its baudrate is known.
*		Note that the channel to be used is channel 'B' and baudrate is set 
*		to 115200.
*		Note that the module extract the DUART channel B base using the CPU 
*		interface API. This is done in order to make this code independed 
*		of its location relative to CPU interface initialization where address
*		decodes are updated.
*
* INPUT:
*		None.
*
* OUTPUT:
*		None.
*
* RETURN:
*		None.
*
*******************************************************************************/
MV_U32 refClkDevInit(MV_VOID)
{
	return 0;
}

/*******************************************************************************
* refClkDevRelease - Release the reference clock device.
*
* DESCRIPTION:
*		This function release the on board DUART device to its previous 
*		state before using it as a reference clocking device.
*		Note that the module extract the DUART channel B base using the CPU 
*		interface API. This is done in order to make this code independed 
*		of its location relative to CPU interface initialization where address
*		decodes are updated.
*
* INPUT:
*       prevState - Previous state with the following encoding:
*                    byte0 - previous IER
*                    byte1 - previous DLL
*                    byte2 - previous DLM
*                    byte3 - previous divisior high
*
* OUTPUT:
*		None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID refClkDevRelease(MV_U32 prevState)
{	
	return;
}

/*******************************************************************************
* refClkDevBitRateGet - Get the reference clock device bit rate.
*
* DESCRIPTION:
*		The UART device bauderate is theoretical. The UART protocol insert 
*		control bits along with data transport (start and stop bits). 
*		This means that for each 8 bit data (we use the UART with 8 bit data)
*		two bits are for control, which gives us actual bauderate ~82.5% of
*		the theoretical one.
*		This function retrieve the on board DUART device bauderate.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_U32 refClkDevBitRateGet(MV_VOID)
{
	return (mvBoardTclkGet() * TCLK_TO_COUNTER_RATIO);	
}

/*******************************************************************************
* refClkDevStart - Start the reference clock device.
*
* DESCRIPTION:
*		This enable counter REF_TIMER for an input number of tikcs.
*
* INPUT:
*       refClkDevBits - Number of bits to transmit (must be a multiple of 8).
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID refClkDevStart(MV_U32 refClkDevBits)
{
    MV_CNTMR_CTRL cntmrCtrl;
    
    cntmrCtrl.enable  = MV_TRUE;
    
	    
    if (MV_OK != mvCntmrStart(REF_TIMER, refClkDevBits, &cntmrCtrl))
    {
	    DB(mvOsPrintf("refClkDevStart - Can't start counter %d\n", REF_TIMER));		
    }
    else
    {
	   while(mvCntmrRead(REF_TIMER));
    }
}

/*******************************************************************************
* mvBoardVoiceAssembleModeGet - return SLIC/DAA assembly & interrupt modes  
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*
*******************************************************************************/

MV_VOID mvBoardVoiceConnModeGet(MV_32* connMode, MV_32* irqMode)
{
	switch(mvBoardIdGet())
	{
		case DB_78XX0_ID:
		case DB_78200_ID:
		case DB_632X_ID:
			 *connMode = DUAL_CHIP_SELECT_MODE;
			 *irqMode = INTERRUPT_TO_TDM;
			break;
		default:
			*connMode = *irqMode = -1;
			mvOsPrintf("mvBoardVoiceAssembleModeGet: TDM not supported(boardId=0x%x)\n",mvBoardIdGet());
	}
		return;
	
}
/*******************************************************************************
* mvBoardTdmMppSet - set MPPs in TDM module
*
* DESCRIPTION:
*
* INPUT: type of second telephony device
*
* OUTPUT:
*       None.
*
* RETURN:
*
*******************************************************************************/
MV_VOID mvBoardTdmMppSet(MV_32 chType)
{
	return;
}
/*******************************************************************************
* mvBoardSlicGpioPinGet -
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*
*
*******************************************************************************/
MV_32 mvBoardSlicGpioPinGet(MV_U32 slicNum)
{
	MV_U32 boardId;
	boardId = mvBoardIdGet();

	switch (boardId)
	{
		case DB_78XX0_ID:
		case DB_78200_ID:
		case DB_632X_ID:
		default:
			return MV_ERROR;		
	}
}

/*******************************************************************************
* mvBoardTdmSpiModeGet - return SLIC/DAA connection 
*
* DESCRIPTION:
*
* INPUT:
*
* OUTPUT:
*       None.
*
* RETURN:
*
*******************************************************************************/
MV_32 mvBoardTdmSpiModeGet(MV_VOID)
{
	return DUAL_CHIP_SELECT_MODE;	
}

