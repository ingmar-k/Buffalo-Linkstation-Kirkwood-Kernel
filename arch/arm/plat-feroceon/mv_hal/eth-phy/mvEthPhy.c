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

#include "eth-phy/mvEthPhy.h"
#include "eth/gbe/mvEthRegs.h"
#include "boardEnv/mvBoardEnvLib.h"

static 	MV_VOID	mvEthPhyPower(MV_U32 ethPortNum, MV_BOOL enable);
void    rdPhy(MV_U32 phyAddr, MV_U32 regOffs);


void    rdPhy(MV_U32 phyAddr, MV_U32 regOffs)
{
    MV_U16      data;
    MV_STATUS   status;

    status = mvEthPhyRegRead(phyAddr, regOffs, &data);
    if(status == MV_OK)
        mvOsPrintf("reg=%d: 0x%04x\n", regOffs, data);
    else
        mvOsPrintf("Read failed\n");
}


/*******************************************************************************
* mvEthPhyRegRead - Read from ethernet phy register.
*
* DESCRIPTION:
*       This function reads ethernet phy register.
*
* INPUT:
*       phyAddr - Phy address.
*       regOffs - Phy register offset.
*
* OUTPUT:
*       None.
*
* RETURN:
*       16bit phy register value, or 0xffff on error
*
*******************************************************************************/
MV_STATUS mvEthPhyRegRead(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 *data)
{
	MV_U32 			smiReg;
	volatile MV_U32 timeout;

	/* check parameters */
	if ((phyAddr << ETH_PHY_SMI_DEV_ADDR_OFFS) & ~ETH_PHY_SMI_DEV_ADDR_MASK)
	{
		mvOsPrintf("mvEthPhyRegRead: Err. Illegal PHY device address %d\n",
                   phyAddr);
		return MV_FAIL;
	}
	if ((regOffs <<  ETH_PHY_SMI_REG_ADDR_OFFS) & ~ETH_PHY_SMI_REG_ADDR_MASK)
	{
		mvOsPrintf("mvEthPhyRegRead: Err. Illegal PHY register offset %d\n", 
                   regOffs);
		return MV_FAIL;
	}

	timeout = ETH_PHY_TIMEOUT;
	/* wait till the SMI is not busy*/
	do
	{
		/* read smi register */
		smiReg = MV_REG_READ(ETH_PHY_SMI_REG);
		if (timeout-- == 0) 
		{
			mvOsPrintf("mvEthPhyRegRead: SMI busy timeout\n");
            		return MV_FAIL;
		}
	}while (smiReg & ETH_PHY_SMI_BUSY_MASK);

	/* fill the phy address and regiser offset and read opcode */
	smiReg = (phyAddr <<  ETH_PHY_SMI_DEV_ADDR_OFFS) | (regOffs << ETH_PHY_SMI_REG_ADDR_OFFS )|
			   ETH_PHY_SMI_OPCODE_READ;

	/* write the smi register */
	MV_REG_WRITE(ETH_PHY_SMI_REG, smiReg);

	timeout=ETH_PHY_TIMEOUT;

	/*wait till readed value is ready */
	do
	{
		/* read smi register */
		smiReg=MV_REG_READ(ETH_PHY_SMI_REG);

		if (timeout-- == 0) {
			mvOsPrintf("mvEthPhyRegRead: SMI read-valid timeout\n");
            		return MV_FAIL;
		}
	}while (!(smiReg & ETH_PHY_SMI_READ_VALID_MASK));

    	/* Wait for the data to update in the SMI register */
	for(timeout = 0 ; timeout < ETH_PHY_TIMEOUT ; timeout++);
	
	*data = (MV_U16)( MV_REG_READ(ETH_PHY_SMI_REG) & ETH_PHY_SMI_DATA_MASK);

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyRegWrite - Write to ethernet phy register.
*
* DESCRIPTION:
*       This function write to ethernet phy register.
*
* INPUT:
*       phyAddr - Phy address.
*       regOffs - Phy register offset.
*       data    - 16bit data.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK if write succeed, MV_BAD_PARAM on bad parameters , MV_ERROR on error .
*		MV_TIMEOUT on timeout 
*
*******************************************************************************/
MV_STATUS mvEthPhyRegWrite(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 data)
{
	MV_U32 			smiReg;
	volatile MV_U32 timeout;

	/* check parameters */
	if ((phyAddr <<  ETH_PHY_SMI_DEV_ADDR_OFFS) & ~ETH_PHY_SMI_DEV_ADDR_MASK)
	{
		mvOsPrintf("mvEthPhyRegWrite: Err. Illegal phy address \n");
		return MV_BAD_PARAM;
	}
	if ((regOffs <<  ETH_PHY_SMI_REG_ADDR_OFFS) & ~ETH_PHY_SMI_REG_ADDR_MASK)
	{
		mvOsPrintf("mvEthPhyRegWrite: Err. Illegal register offset \n");
		return MV_BAD_PARAM;
	}
	
	timeout=ETH_PHY_TIMEOUT;

	/* wait till the SMI is not busy*/
	do
	{
		/* read smi register */
		smiReg=MV_REG_READ(ETH_PHY_SMI_REG);
		if (timeout-- == 0) {
			mvOsPrintf("mvEthPhyRegWrite: SMI busy timeout\n");
            return MV_TIMEOUT;
		}
	}while (smiReg & ETH_PHY_SMI_BUSY_MASK);

	/* fill the phy address and regiser offset and write opcode and data*/
	smiReg = (data << ETH_PHY_SMI_DATA_OFFS);
	smiReg |= (phyAddr <<  ETH_PHY_SMI_DEV_ADDR_OFFS) | (regOffs << ETH_PHY_SMI_REG_ADDR_OFFS );
    smiReg &= ~ETH_PHY_SMI_OPCODE_READ;

	/* write the smi register */
	MV_REG_WRITE(ETH_PHY_SMI_REG, smiReg);

	return MV_OK;


}

/*******************************************************************************
* mvEthPhyReset - Reset ethernet Phy.
*
* DESCRIPTION:
*       This function resets a given ethernet Phy.
*
* INPUT:
*       phyAddr - Phy address.
*       timeout - in millisec        
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK       - Success
*           MV_TIMEOUT  - Timeout 
*       
*******************************************************************************/
MV_STATUS mvEthPhyReset(MV_U32 phyAddr, int timeout)
{
  	MV_U16  phyRegData;

    /* Reset the PHY */
    if(mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;		 

    /* Set bit 15 to reset the PHY */
	phyRegData |= ETH_PHY_CTRL_RESET_MASK; 
	mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, phyRegData);

	/* Wait untill Reset completed */
    while(timeout > 0)
	{
        mvOsSleep(100);
        timeout -= 100;

		if( mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
			return MV_FAIL;		

	    if( (phyRegData & ETH_PHY_CTRL_RESET_MASK) == 0)
            return MV_OK;
   	}
	return MV_TIMEOUT;
}


/*******************************************************************************
* mvEthPhyRestartAN - Restart ethernet Phy Auto-Negotiation.
*
* DESCRIPTION:
*       This function resets a given ethernet Phy.
*
* INPUT:
*       phyAddr - Phy address.
*       timeout - in millisec; 0 - no timeout (don't wait)
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK       - Success
*           MV_TIMEOUT  - Timeout 
*
*******************************************************************************/
MV_STATUS mvEthPhyRestartAN(MV_U32 phyAddr, int timeout)
{
  	MV_U16  phyRegData;

    /* Reset the PHY */
     if(mvEthPhyRegRead (phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;

    /* Set bit 12 to Enable autonegotiation of the PHY */
	phyRegData |= ETH_PHY_CTRL_AN_ENABLE_MASK; 

	/* Set bit 9 to Restart autonegotiation of the PHY */
	phyRegData |= ETH_PHY_CTRL_AN_RESTART_MASK; 
	mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, phyRegData);

	if (timeout == 0)
		return MV_OK;

	/* Wait untill Auotonegotiation completed */
    while(timeout > 0)
	{
       	mvOsSleep(100);
       	timeout -= 100;

		if( mvEthPhyRegRead(phyAddr, ETH_PHY_STATUS_REG, &phyRegData) != MV_OK)
			return MV_FAIL;
	
        if(phyRegData & ETH_PHY_STATUS_AN_DONE_MASK)
       		return MV_OK;
    }
    return MV_TIMEOUT;
}


/*******************************************************************************
* mvEthPhyDisableAN - Disable Phy Auto-Negotiation and set forced Speed and Duplex
*
* DESCRIPTION:
*       This function disable AN and set duplex and speed.
*
* INPUT:
*       phyAddr - Phy address.
*       speed   - port speed. 0 - 10 Mbps, 1-100 Mbps, 2 - 1000 Mbps        
*       duplex  - port duplex. 0 - Half duplex, 1 - Full duplex
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*
*******************************************************************************/
MV_STATUS mvEthPhyDisableAN(MV_U32 phyAddr, int speed, int duplex)
{
  	MV_U16  phyRegData;

  	if(mvEthPhyRegRead (phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;

    switch(speed)
    {
        case 0: /* 10 Mbps */
            phyRegData &= ~ETH_PHY_CTRL_SPEED_LSB_MASK;
            phyRegData &= ~ETH_PHY_CTRL_SPEED_MSB_MASK;
            break;

        case 1: /* 100 Mbps */
            phyRegData |= ETH_PHY_CTRL_SPEED_LSB_MASK;
            phyRegData &= ~ETH_PHY_CTRL_SPEED_MSB_MASK;
            break;

        case 2: /* 1000 Mbps */
            phyRegData &= ~ETH_PHY_CTRL_SPEED_LSB_MASK;
            phyRegData |= ETH_PHY_CTRL_SPEED_MSB_MASK;
            break;
    
        default:
            mvOsOutput("Unexpected speed = %d\n", speed);
            return MV_FAIL;
    }
    
    switch(duplex)
    {
        case 0: /* half duplex */
            phyRegData &= ~ETH_PHY_CTRL_DUPLEX_MASK;
            break;

        case 1: /* full duplex */
            phyRegData |= ETH_PHY_CTRL_DUPLEX_MASK;
            break;

        default:
            mvOsOutput("Unexpected duplex = %d\n", duplex);
    }
    /* Clear bit 12 to Disable autonegotiation of the PHY */
	phyRegData &= ~ETH_PHY_CTRL_AN_ENABLE_MASK; 

	/* Clear bit 9 to DISABLE, Restart autonegotiation of the PHY */
	phyRegData &= ~ETH_PHY_CTRL_AN_RESTART_MASK; 
	mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, phyRegData);

    return MV_OK;
}

MV_STATUS   mvEthPhyLoopback(MV_U32 phyAddr, MV_BOOL isEnable)
{
    MV_U16      regVal, ctrlVal;
    MV_STATUS   status;

    /* Set loopback speed and duplex accordingly with current */
    /* Bits: 6, 8, 13 */
  	if(mvEthPhyRegRead (phyAddr, ETH_PHY_CTRL_REG, &ctrlVal) != MV_OK)
		return MV_FAIL;

    if(isEnable)
    {
        /* Select page 2 */
        mvEthPhyRegWrite(phyAddr, 22, 2);

        mvEthPhyRegRead (phyAddr, 21, &regVal);
        regVal &= ~(ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_SPEED_LSB_MASK | 
                    ETH_PHY_CTRL_SPEED_MSB_MASK | ETH_PHY_CTRL_AN_ENABLE_MASK);
        regVal |= (ctrlVal & (ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_SPEED_LSB_MASK | 
                            ETH_PHY_CTRL_SPEED_MSB_MASK | ETH_PHY_CTRL_AN_ENABLE_MASK) );
        mvEthPhyRegWrite(phyAddr, 21, regVal);

        /* Select page 0 */
        mvEthPhyRegWrite(phyAddr, 22, 0);
    
        /* Disable Energy detection   R16[9:8] = 00 */
        /* Disable MDI/MDIX crossover R16[6:5] = 00 */
        mvEthPhyRegRead (phyAddr, ETH_PHY_SPEC_CTRL_REG, &regVal);
        regVal &= ~(BIT5 | BIT6 | BIT8 | BIT9);
        mvEthPhyRegWrite (phyAddr, ETH_PHY_SPEC_CTRL_REG, regVal);

        status = mvEthPhyReset(phyAddr, 1000);
        if(status != MV_OK)
        {
            mvOsPrintf("mvEthPhyReset failed: status=0x%x\n", status);
            return status;
        }

        /* Set loopback */
        ctrlVal |= ETH_PHY_CTRL_LOOPBACK_MASK;
        mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, ctrlVal);
    }
    else
    {
        /* Cancel Loopback */
        ctrlVal &= ~ETH_PHY_CTRL_LOOPBACK_MASK;
        mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, ctrlVal);
        
        status = mvEthPhyReset(phyAddr, 1000);
        if(status != MV_OK)
        {
            mvOsPrintf("mvEthPhyReset failed: status=0x%x\n", status);
            return status;
        }
        
        /* Enable Energy detection   R16[9:8] = 11 */
        /* Enable MDI/MDIX crossover R16[6:5] = 11 */
        mvEthPhyRegRead (phyAddr, ETH_PHY_SPEC_CTRL_REG, &regVal);
        regVal |= (BIT5 | BIT6 | BIT8 | BIT9);
        mvEthPhyRegWrite (phyAddr, ETH_PHY_SPEC_CTRL_REG, regVal);

        status = mvEthPhyReset(phyAddr, 1000);
        if(status != MV_OK)
        {
            mvOsPrintf("mvEthPhyReset failed: status=0x%x\n", status);
            return status;
        }
    }

    return MV_OK;
}

/*******************************************************************************
* mvEthPhyCheckLink - 
*
* DESCRIPTION:
*	check link in phy port
*       
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:   MV_TRUE if link is up, MV_FALSE if down
*
*******************************************************************************/
MV_BOOL mvEthPhyCheckLink( MV_U32 phyAddr )
{
	MV_U16 val_st, val_ctrl, val_spec_st;

	/* read status reg */
	if( mvEthPhyRegRead( phyAddr, ETH_PHY_STATUS_REG, &val_st) != MV_OK )
		return MV_FALSE;

	/* read control reg */
	if( mvEthPhyRegRead( phyAddr, ETH_PHY_CTRL_REG, &val_ctrl) != MV_OK )
		return MV_FALSE;

	/* read special status reg */
	if( mvEthPhyRegRead( phyAddr, ETH_PHY_SPEC_STATUS_REG, &val_spec_st) != MV_OK )
		return MV_FALSE;

	/* Check for PHY exist */
	if((val_ctrl == ETH_PHY_SMI_DATA_MASK) && (val_st & ETH_PHY_SMI_DATA_MASK))
		return MV_FALSE;


	if(val_ctrl & ETH_PHY_CTRL_AN_ENABLE_MASK)
	{
		if(val_st & ETH_PHY_STATUS_AN_DONE_MASK)
			return MV_TRUE;
		else
			return MV_FALSE;
	}
	else
	{
		if(val_spec_st & ETH_PHY_SPEC_STATUS_LINK_MASK)
			return MV_TRUE;
	}
	return MV_FALSE;
}

/*******************************************************************************
* mvEthPhyPrintStatus - 
*
* DESCRIPTION:
*	print port Speed, Duplex, Auto-negotiation, Link.
*       
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:   16bit phy register value, or 0xffff on error
*
*******************************************************************************/
MV_STATUS	mvEthPhyPrintStatus( MV_U32 phyAddr )
{
	MV_U16 val;

	/* read control reg */ 
	if( mvEthPhyRegRead( phyAddr, ETH_PHY_CTRL_REG, &val) != MV_OK )
		return MV_ERROR;

	if( val & ETH_PHY_CTRL_AN_ENABLE_MASK )
		mvOsOutput( "Auto negotiation: Enabled\n" );
	else
		mvOsOutput( "Auto negotiation: Disabled\n" );


	/* read specific status reg */ 
	if( mvEthPhyRegRead( phyAddr, ETH_PHY_SPEC_STATUS_REG, &val) != MV_OK )
		return MV_ERROR;

	switch (val & ETH_PHY_SPEC_STATUS_SPEED_MASK)
	{
		case ETH_PHY_SPEC_STATUS_SPEED_1000MBPS:
			mvOsOutput( "Speed: 1000 Mbps\n" );
			break;
		case ETH_PHY_SPEC_STATUS_SPEED_100MBPS:
			mvOsOutput( "Speed: 100 Mbps\n" );
			break;
		case ETH_PHY_SPEC_STATUS_SPEED_10MBPS:
			mvOsOutput( "Speed: 10 Mbps\n" );
		default:
			mvOsOutput( "Speed: Uknown\n" );
			break;

	}

	if( val & ETH_PHY_SPEC_STATUS_DUPLEX_MASK )
		mvOsOutput( "Duplex: Full\n" );
	else
		mvOsOutput( "Duplex: Half\n" );
 

	if( val & ETH_PHY_SPEC_STATUS_LINK_MASK )
		mvOsOutput("Link: up\n");
	else
		mvOsOutput("Link: down\n");

	return MV_OK;
}

/*******************************************************************************
* mvEthE1111PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1111PhyBasicInit(MV_U32 ethPortNum)
{
	MV_U16 reg;
	MV_U32 regOff, data;

	/* Phy recv and tx delay */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),20,&reg);
	reg |= BIT1 | BIT7;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),20,reg);

	/* Leds link and activity*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),24,0x4111);

	/* reset the phy */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),0,&reg);
	reg |= BIT15;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,reg);

	if(mvBoardSpecInitGet(&regOff, &data) == MV_TRUE)
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),regOff , data);

}

/*******************************************************************************
* mvEthE1112PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1112PhyBasicInit(MV_U32 ethPortNum)
{
	MV_U16 reg;

	/* Set phy address */
	/*MV_REG_WRITE(ETH_PHY_ADDR_REG(ethPortNum), mvBoardPhyAddrGet(ethPortNum));*/

	/* Implement PHY errata */
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,2);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,0x140);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,0x8140);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0);
	
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,3);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,0x103);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0);

	/* reset the phy */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),0,&reg);
	reg |= BIT15;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,reg);

}

/*******************************************************************************
* mvEthE1116PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1116PhyBasicInit(MV_U32 ethPortNum)
{
	MV_U16 reg;

	/* Set phy address */
	MV_REG_WRITE(ETH_PHY_ADDR_REG(ethPortNum), mvBoardPhyAddrGet(ethPortNum));

	/* Leds link and activity*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0x3);
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),16,&reg);
	reg &= ~0xf;
	reg	|= 0x1;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,reg);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0x0);

	/* Set RGMII delay */
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,2);
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),21,&reg);
	reg	|= (BIT5 | BIT4);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),21,reg);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0);

	/* reset the phy */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),0,&reg);
	reg |= BIT15;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,reg);
}


/*******************************************************************************
* mvEthE1310PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1310PhyBasicInit(MV_U32 ethPortNum)
{
	MV_U16 reg;

	/* Set phy address */
	MV_REG_WRITE(ETH_PHY_ADDR_REG(ethPortNum), mvBoardPhyAddrGet(ethPortNum));

	/* Leds link and activity*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0x3);
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),16,&reg);
	reg &= ~0xf;
	reg |= 0x11;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,reg);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0x0);
}
/*******************************************************************************
* mvEthE3016PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE3016PhyBasicInit(MV_U32 ethPortNum)
{
	MV_U16 reg;

	/* Set phy address */
	MV_REG_WRITE(ETH_PHY_ADDR_REG(ethPortNum), mvBoardPhyAddrGet(ethPortNum));

	/* Leds link and activity*/
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),22,&reg);
	reg &= ~0xf;
	reg	|= 0xa;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,reg);

	/* Set RGMII (RX) delay and copper mode */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),28,&reg);
	reg &= ~(BIT3 | BIT10 | BIT11);
    reg	|= (BIT10);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),28,reg);

	/* reset the phy */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),0,&reg);
	reg |= BIT15;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,reg);
}


/*******************************************************************************
* mvEthE1011PhyBasicInit - 
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1011PhyBasicInit(MV_U32 ethPortNum)
{
	MV_U16 reg;

	/* Phy recv and tx delay */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),20,&reg);
	reg &= ~(BIT1 | BIT7);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),20,reg);

	/* Leds link and activity*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),24,0x4111);

	/* reset the phy */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),0, &reg);
	reg |= BIT15;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,reg);

}

/*******************************************************************************
* mvEthE1112PhyPowerDown - 
*
* DESCRIPTION:
*	Power down the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID	mvEthE1112PhyPowerDown(MV_U32 ethPortNum)
{
	mvEthPhyPower(ethPortNum, MV_FALSE);
}

/*******************************************************************************
* mvEthE1112PhyPowerUp - 
*
* DESCRIPTION:
*	Power up the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID	mvEthE1112PhyPowerUp(MV_U32 ethPortNum)
{
	mvEthPhyPower(ethPortNum, MV_TRUE);
}

/*******************************************************************************
* mvEthPhyPower - 
*
* DESCRIPTION:
*	Do a basic power down/up to the Phy , including reset
*       
* INPUT:
*       ethPortNum - Ethernet port number
*	enable - MV_TRUE - power up
*		 MV_FALSE - power down
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
static MV_VOID	mvEthPhyPower(MV_U32 ethPortNum, MV_BOOL enable)
{
	MV_U16 reg;
	if (enable == MV_FALSE)
	{
	/* Power down command */
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,2); 		/* select page 2 */
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),16,&reg); 		
		reg |= BIT3;								        
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,reg);		/* select to disable the SERDES */	
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0); 		/* select page 0 */			
		
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,3);		/* Power off LED's */
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,0x88);
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0);

		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),ETH_PHY_CTRL_REG,&reg); 		
		reg |= ETH_PHY_CTRL_RESET_BIT;								        
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),ETH_PHY_CTRL_REG,reg);	/* software reset */			
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),ETH_PHY_CTRL_REG,&reg); 		
		reg |= ETH_PHY_CTRL_POWER_DOWN_BIT;								        
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),ETH_PHY_CTRL_REG,reg);	/* power down the PHY */
	}
	else
	{
	/* Power up command */
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,2); 		/* select page 2 */
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),16,&reg); 		
		reg &= ~BIT3;								        
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,reg);		/* select to enable the SERDES */	
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0); 		/* select page 0 */
		
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,3);		/* Power on LED's */
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,0x03);
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0);

		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),ETH_PHY_CTRL_REG,&reg); 		
		reg |= ETH_PHY_CTRL_RESET_BIT;								        
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),ETH_PHY_CTRL_REG,reg);	/* software reset */			
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),ETH_PHY_CTRL_REG,&reg); 		
		reg &= ~ETH_PHY_CTRL_POWER_DOWN_BIT;								        
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),ETH_PHY_CTRL_REG,reg);	/* power up the PHY */
	}
}


/*******************************************************************************
* mvEth1145PhyInit - Initialize MARVELL 1145 Phy 
*
* DESCRIPTION:
*
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvEth1145PhyBasicInit(MV_U32 port)
{
    MV_U16 value;

    /* Set phy address for each port */
    MV_REG_WRITE(ETH_PHY_ADDR_REG(port), mvBoardPhyAddrGet(port));
	    /* Set Link1000 output pin to be link indication, set Tx output pin to be activity */
    mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x18, ETH_PHY_LED_ACT_LNK_DV);
    mvOsDelay(10);
	    
	    /* Add delay to RGMII Tx and Rx */
    mvEthPhyRegRead(mvBoardPhyAddrGet(port), 0x14, &value);
    mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x14,(value | BIT1 | BIT7));
    mvOsDelay(10);
#if 0 /* Fix by yotam */
    if (boardId != RD_78XX0_AMC_ID && 
	    boardId != RD_78XX0_H3C_ID) {
	    /* Set port 2 - Phy addr 9 to RGMII */
	if (port == 2)
	{
		mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x1b, 0x808b);
		mvOsDelay(10);
	}
	
	/* Set port 1 - Phy addr a to SGMII */
	if (port == 1)
	{
	    mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x1b, 0x8084);
	    mvOsDelay(10);
		
		/* Reset Phy */
	    mvEthPhyRegRead( mvBoardPhyAddrGet(port), 0x00, &value);
	    mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x00, (value | BIT15));
	    mvOsDelay(10);
	#if defined(SGMII_OUTBAND_AN)
		/* Set port 1 - Phy addr A Page 1 */
		mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x16, 0x1);
		mvOsDelay(10);
	
		/* Set port 1 - Phy addr A disable A.N. */
		mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x0, 0x140);
		mvOsDelay(10);
	
		/* Set port 1 - Phy addr A reset */
		mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x0, 0x8140);
		mvOsDelay(10);
	
		mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x16, 0x0);
		mvOsDelay(10);
	#endif
	}
    }
#endif

	    /* Set Phy TPVL to 0 */
    mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x10, 0x60);
    mvOsDelay(10);

    /* Reset Phy */
    mvEthPhyRegRead(mvBoardPhyAddrGet(port), 0x00, &value);
    mvEthPhyRegWrite(mvBoardPhyAddrGet(port), 0x00, (value | BIT15));
    mvOsDelay(10);

    return;
}


/*******************************************************************************
* mvEthSgmiiToCopperPhyInit - Initialize Test board 1112 Phy 
*
* DESCRIPTION:
*
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvEthSgmiiToCopperPhyBasicInit(MV_U32 port)
{
    MV_U16 value;    
    MV_U16 phyAddr = 0xC;
    
   /* Port 0 phyAdd c */   
   /* Port 1 phyAdd d */   
    mvEthPhyRegWrite(phyAddr + port,22,3);
    mvEthPhyRegWrite(phyAddr + port,16,0x103);
    mvEthPhyRegWrite(phyAddr + port,22,0);

		/* reset the phy */
    mvEthPhyRegRead(phyAddr + port,0,&value);
    value |= BIT15;
    mvEthPhyRegWrite(phyAddr + port,0,value);
}


MV_VOID mvEth1121PhyBasicInit(MV_U32 port)
{
    	MV_U16 value;
	MV_U16 phyAddr = mvBoardPhyAddrGet(port);

	MV_REG_WRITE(ETH_PHY_ADDR_REG(port), phyAddr);
	
	/* Change page select to 2 */
	value = 2;
	mvEthPhyRegWrite(phyAddr, 22, value);
    	mvOsDelay(10);

	/* Set RGMII rx delay */
	mvEthPhyRegRead(phyAddr, 21, &value);
	value |= BIT5;
	mvEthPhyRegWrite(phyAddr, 21, value);
    	mvOsDelay(10);

	/* Change page select to 0 */
	value = 0;
	mvEthPhyRegWrite(phyAddr, 22, value);
    	mvOsDelay(10);

	/* reset the phy */
	mvEthPhyRegRead(phyAddr, 0, &value);
	value |= BIT15;
	mvEthPhyRegWrite(phyAddr, 0, value);
    	mvOsDelay(10);
}

#if defined(CONFIG_BUFFALO_USE_GPIO_DRIVER)
MV_VOID mvEthE111xPhyBasicInit_internal(MV_U32 ethPortNum, MV_U32 flag_PolarityChange)
{
	MV_U16 reg;

	/*read PHY Ident2*/
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 3, &reg);
	reg &= 0x03f0 ;

	if(reg == 0x00c0) {
		/* Phy recv and tx delay */
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),20,&reg);
		reg |= BIT1 | BIT7;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),20,reg);

		/* Leds link and activity*/
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),24,0x4111);

		/* reset the phy */
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),0,&reg);
		reg |= BIT15;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,reg);
		return;
	} else {
		/* Added by Hiroshi Tokoyo */
		MV_REG_WRITE(0xf1072000, mvBoardPhyAddrGet(ethPortNum));
		/* Access Page 3*/
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0x3);
		reg = 0x0911;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,reg);

		if (flag_PolarityChange) {
			// led polarity setting.
			mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 17, &reg);
			reg &= ~0x003f;
			reg |= 0x0005;
			mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 17, reg);
		}

		/* change to Page 0*/
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0x0);
		/* Added by Hiroshi */
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),0,&reg);
		reg |= 0x8000;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),0,reg);
		return;
	}
}

/*******************************************************************************
* mvEthE1111PhyBasicTest -
*
* DESCRIPTION:
*
*
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       .
*
* RETURN:   None
*
*******************************************************************************/
MV_STATUS Is_mvEthE111xPhy(MV_U32 ethPortNum)
{
	MV_U16 reg;

	/* Phy recv and tx delay */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),2,&reg);
	/*Check EthE1111PhyID */
        if (reg == 0x141) { /*Identify Bit*/
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),3,&reg);
		reg &= 0x03f0;
		if(reg == 0x00c0)
			return 0x1111;
		else
			if(reg == 0x0210)
				return 0x1118;
		return MV_OK;
	} else
		return MV_FAIL;
}

/*******************************************************************************
* mvEthE1111PhyBasicTest -
*
* DESCRIPTION:
*
*
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       .
*
* RETURN:   None
*
*******************************************************************************/
MV_STATUS Is_link_mvEthE1111Phy(MV_U32 ethPortNum)
{
	MV_U16 val;

	/* Phy recv and tx delay */
	if(mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), ETH_PHY_SPEC_STATUS_REG, &val) != MV_OK ) {
		/* return MV_ERROR; */
	}

	if((val & 0x0400) == 0x0400 ) /*is LINK UP ?*/
		return MV_OK;
	else
		return MV_FAIL;
}

// 1112, 1118 common.
MV_VOID
buffalo_link_led_off_mvEthE111xPhy(MV_U32 ethPortNum)
{
	MV_U16 reg;

#if 0
	// null pointer error occured.
	MV_U32 CurrentPageBackup = mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 22, 0);
#endif

	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 3);

	// led work mode setting.
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 16, &reg);
	reg &= 0xFFF0;
	reg |= 0x0008;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 16, reg);
#if 0
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, CurrentPageBackup);
#else
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 0);
#endif
}

// 1112, 1118 common.
MV_VOID
buffalo_link_led_on_mvEthE111xPhy(MV_U32 ethPortNum)
{
	MV_U16 reg;

#if 0
	MV_U32 CurrentPageBackup = mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 22, 0);
#endif

	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 3);
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 16, &reg);
	reg &= 0xFFF0;
	reg |= 0x0001;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 16, reg);
#if 0
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, CurrentPageBackup);
#else
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 0);
#endif
}

MV_VOID
buffalo_link_led_off(MV_U32 ethPortNum)
{

	buffalo_link_led_off_mvEthE111xPhy(ethPortNum);
}

MV_VOID
buffalo_link_led_on(MV_U32 ethPortNum)
{

	buffalo_link_led_on_mvEthE111xPhy(ethPortNum);
}

MV_VOID
buffalo_print_phy_addr(MV_VOID)
{
	MV_U32 port;

	for (port = 0; port < mvCtrlEthMaxPortGet(); port++) {
		printk("PHY %d :0x%08x\n", port, mvBoardPhyAddrGet(port));
		mvEthPhyPrintStatus(mvBoardPhyAddrGet(port));
		printk("\n");
	}
}
#endif // end of CONFIG_BUFFALO_USE_GPIO_DRIVER
