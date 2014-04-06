#if defined(__KERNEL__) && !defined(MV_UBOOT)
/* include file on kernel */
#include <linux/version.h>
#include <linux/if_ether.h>

#include "BuffaloGpio.h"
#include "eth-phy/mvEthPhy.h"
#include "eth-phy/mvEthPhyRegs.h"
#include "eth/mvEth.h"
#include "eth/gbe/mvEthGbe.h"
#include "eth/gbe/mvEthRegs.h"
#include "BuffaloWol.h"
#else
/* include file on u-boot */
#include "eth-phy/mvEthPhy.h"
#include "eth-phy/mvEthPhyRegs.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "eth/mvEth.h"
#include "eth/gbe/mvEthGbe.h"
#include "eth/gbe/mvEthRegs.h"
#include "BuffaloWol.h"

#include <net.h>
#endif


#if defined(__KERNEL__) && !defined(MV_UBOOT)
#define DEBUG(fmt, args...)	printk(fmt, ##args)
typedef unsigned char uchar;

/*
 *      Ethernet header
 */
typedef struct {
	uchar           et_dest[6];     /* Destination node             */
	uchar           et_src[6];      /* Source node                  */
	ushort          et_protlen;     /* Protocol or length           */
	uchar           et_dsap;        /* 802 DSAP                     */
	uchar           et_ssap;        /* 802 SSAP                     */
	uchar           et_ctl;         /* 802 control                  */
	uchar           et_snap1;       /* SNAP                         */
	uchar           et_snap2;
	uchar           et_snap3;
	ushort          et_prot;        /* 802 protocol                 */
} Ethernet_t;

#define ETHER_HDR_SIZE  14              /* Ethernet header size         */
#define E802_HDR_SIZE   22              /* 802 ethernet header size     */

typedef ulong           IPaddr_t;

/*
 *      Internet Protocol (IP) header.
 */
typedef struct {
	uchar           ip_hl_v;        /* header length and version    */
	uchar           ip_tos;         /* type of service              */
	ushort          ip_len;         /* total length                 */
	ushort          ip_id;          /* identification               */
	ushort          ip_off;         /* fragment offset field        */
	uchar           ip_ttl;         /* time to live                 */
	uchar           ip_p;           /* protocol                     */
	ushort          ip_sum;         /* checksum                     */
	IPaddr_t        ip_src;         /* Source IP address            */
	IPaddr_t        ip_dst;         /* Destination IP address       */
	ushort          udp_src;        /* UDP source port              */
	ushort          udp_dst;        /* UDP destination port         */
	ushort          udp_len;        /* Length of UDP packet         */
	ushort          udp_xsum;       /* Checksum                     */
} IP_t;

#define IP_HDR_SIZE_NO_UDP      (sizeof (IP_t) - 8)
#define IP_HDR_SIZE             (sizeof (IP_t))

#else
#define DEBUG(fmt, args...)	printf(fmt, ##args)
#endif

#define FLAG_BUFFALO_WOL_UCAST          (0 << 0)
#define FLAG_BUFFALO_WOL_BCAST          (1 << 0)
#define FLAG_BUFFALO_WITH_UDP_H         (0 << 1)
#define FLAG_BUFFALO_WITHOUT_UDP_H      (1 << 1)
#define FLAG_BUFFALO_DISACTIVE_PACKET   (0 << 2)
#define FLAG_BUFFALO_ACTIVE_PACKET      (1 << 2)

struct bfWolSupportPhyList_st {
	MV_U32 OUI;
	MV_U16 ModelNumber;
};

struct bfWolInfo_st{
	char name[32];
	MV_U8 packet_type;
	MV_U16 port_num;
};

static const struct bfWolSupportPhyList_st bfWolSupportPhyList[] = {
	{0x005043, 0x29},               /* Marvell, 1318 */
};

static const struct bfWolInfo_st bfWolPacketType[] = {
	{"active", (FLAG_BUFFALO_ACTIVE_PACKET | FLAG_BUFFALO_WOL_BCAST), 0x0009}, /* broadcast active packet */
	{"active", (FLAG_BUFFALO_ACTIVE_PACKET), 0x0009}, /* unicast active packet */
	{"wol", (FLAG_BUFFALO_ACTIVE_PACKET | FLAG_BUFFALO_WOL_BCAST), 0x0900}, /* broadcast wol packet */
	{"wol", (FLAG_BUFFALO_ACTIVE_PACKET), 0x0900}, /* unicast wol packet */
	{"wol", (FLAG_BUFFALO_ACTIVE_PACKET | FLAG_BUFFALO_WITHOUT_UDP_H), 0}, /* unicast udp header less wol packet */
	{"wol", (FLAG_BUFFALO_ACTIVE_PACKET | FLAG_BUFFALO_WOL_BCAST | FLAG_BUFFALO_WITHOUT_UDP_H), 0}, /* broadcast udp header less wol packet */
};

#define MAX_WOL_PHY_LIST        ((sizeof(bfWolSupportPhyList)) / (sizeof(bfWolSupportPhyList[0])))
#define MAX_WOL_TYPE    ((sizeof(bfWolPacketType)) / (sizeof(bfWolPacketType[0])))

/* used for return buffer of bfGetWolPatternString */
static char bfWolPatternString[128];

#if 0
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
#endif



/*******************************************************************************
* bfIsSupportWol
*
* DESCRIPTION:
*       Check specified ethPortNum support WOL or not.
*
* INPUT:
*       ethPortNum : port No. to check.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE  : Specified eth port suports WOL function.
*       MV_FALSE : Specified eth port doesn't support WOL function.
*
*******************************************************************************/
MV_BOOL
bfIsSupportWol(MV_U32 ethPortNum)
{
	MV_U8 i = 0;
	MV_U16 old_page;
	MV_U16 reg;
	MV_U32 OUI = 0;
	MV_U16 ModelNumber = 0;

	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 22, &old_page);

	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 0);
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 2, &reg);
	OUI |= (MV_U32)(reg << 6);
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 3, &reg);
	OUI |= ((reg & 0xFC00) >> 10);
	ModelNumber = ((reg & 0x03F0) >> 4);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, old_page);

	for(i = 0; i < MAX_WOL_PHY_LIST; i++)
	{
		if(bfWolSupportPhyList[i].OUI == OUI && bfWolSupportPhyList[i].ModelNumber == ModelNumber)
			return MV_TRUE;
	}
	return MV_FALSE;
}

/*******************************************************************************
* bfResetEthPhy
*
* DESCRIPTION:
*       Do reset eth phy.
*
* INPUT:
*       ethPortNum : port No. to to reset.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
void
bfResetEthPhy(MV_U32 ethPortNum)
{
	DEBUG("Resetting EthPhy\n");
	mvEthPhyReset(mvBoardPhyAddrGet(ethPortNum), 1000);
}

/*******************************************************************************
* bfControlWolInterrupt
*
* DESCRIPTION:
*       Control WOL interrupt status (to Enable / Disable).
*
* INPUT:
*       ethPortNum : port No. to control interrupt status.
*       ope        : specify operation mode to do.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
void
bfControlWolInterrupt(MV_U32 ethPortNum, MV_U8 ope)
{
	MV_U16 reg;
	MV_U16 old_page;

	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 22, &old_page);  /* backup present page information */
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,3);   /* change to Page 3 */

	if(ope == FLAG_BUFFALO_WOL_INTERRUPT_ENABLE)
	{
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),16, &reg);
		reg &= ~0x0fff;
		reg |= 0x08AA;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16, reg);	/* change led[0][1] off (force Hi-Z)*/

		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),18, &reg);
		reg |= 0x0880;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),18, reg);	/* change led[2] to intn pin */
	}
	else
	{
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),16, &reg);
		reg &= ~0x0fff;
		reg |= 0x0911;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 16, reg);

		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),18, &reg);
		reg &= ~0x0880;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 18, reg);
	}

	/* change to Page 0*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0x0);
	/*WOL Interrupt Enable*/
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),18, &reg);
	if(ope == FLAG_BUFFALO_WOL_INTERRUPT_ENABLE)
		reg |= 0x80 ;
	else
		reg &= ~(0x80);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),18,reg);

	if(ope == FLAG_BUFFALO_WOL_INTERRUPT_ENABLE)
		mvEthPhyDisableAN(mvBoardPhyAddrGet(ethPortNum), 0, 0);
	else
		mvEthPhyRestartAN(mvBoardPhyAddrGet(ethPortNum), 1000);

	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, old_page);  /* restore previous page */
}

/*******************************************************************************
* bfSetPowerSaveMode
*
* DESCRIPTION:
*       Do settings to reduce power consumption during WOL mode.
*
* INPUT:
*       ethPortNum : port No. to set to WOL mode.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
void
bfSetPowerSaveMode(MV_U32 ethPortNum)
{
	MV_U16  reg;

	/*RGMII BUS treansistor OFF*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,0x2); /* change to Page 2 */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),24, &reg);
	reg &= ~(BIT8 | BIT9 | BIT10 | BIT2 | BIT1 | BIT0);
	reg |=  BIT6 ;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),24,reg);

	/*STOP CLK125 OUTPUT */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 16, &reg);
	reg |= ( BIT2 | BIT1 | BIT11| BIT12);
	reg &= ~BIT3 ;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 16, reg);
	/* change to Page 0*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 0);

#if defined(energy_detect)
	/*RGMII STOP for Power Saving */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 16, &reg);
	reg |=  (BIT2 | BIT8 | BIT9) ;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 16, reg);
#endif
}


/*******************************************************************************
* bfSetWolMagicWaitMode
*
* DESCRIPTION:
*       Set specified ethernet port to Wake on Lan waiting mode.
*
* INPUT:
*       ethPortNum : port No. to set to WOL mode.
*       phyAddr    : Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_BOOL
bfSetWolMagicWaitMode(MV_U32 ethPortNum)
{
	MV_U16  reg ;
	unsigned char MacAddr[6];

	if(bfIsSupportWol(ethPortNum) == MV_FALSE)
		return MV_FALSE;

	memset(MacAddr, 0, sizeof(MacAddr));
	mvEthMacAddrGet(ethPortNum, MacAddr);

	/* Set WOL mode */
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,17);  /* change to Page 17 */
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,0x5500);      /* 0101 0101 0000 0000 */

	/* set wol mac address from env data */
	reg = (MacAddr[4]) | (MacAddr[5] << 8) ;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),23,reg);
	reg = (MacAddr[2]) | (MacAddr[3] << 8) ;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),24,reg);
	reg = (MacAddr[0]) | (MacAddr[1] << 8) ;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),25,reg);

	/*Enable Magic Packet*/
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16,0x5500);

	bfControlWolInterrupt(ethPortNum, FLAG_BUFFALO_WOL_INTERRUPT_ENABLE);
	bfSetPowerSaveMode(ethPortNum);
	return MV_TRUE;
}

/*******************************************************************************
* bfSetFrameEventWol
*
* DESCRIPTION:
*       generate frame pattern used to matching.
*
* INPUT:
*       frame    : buffer to store generated frame pattern.
*       mask     : buffer to stora generated mask pattern.
*       pmacAddr : MAC address to generate frame pattern.
*       bfWPT    : flags to generate frame pattern.
*       debug    : debug flag.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
static MV_U8
bfSetFrameEventWol(MV_U8 *frame, MV_U8 *mask, MV_U8 *pMacAddr, const struct bfWolInfo_st *bfWPT, int debug)
{
	MV_U8 i;
	MV_U8 magic_len = 0;
	MV_U8 frame_match_len = 0;

	Ethernet_t *eth_h = (Ethernet_t *)&frame[0];
	Ethernet_t *eth_h_mask = (Ethernet_t *)&mask[0];

	IP_t *ip_h = (IP_t *)&frame[ETHER_HDR_SIZE];
	IP_t *ip_h_mask = (IP_t *)&mask[ETHER_HDR_SIZE];

	MV_U8 *data = NULL;
	MV_U8 *data_mask = NULL;

	for(i = 0; i < 6; i++)
	{
		if(bfWPT->packet_type & FLAG_BUFFALO_WOL_BCAST)
			eth_h->et_dest[i] = 0xFF;
		else
			eth_h->et_dest[i] = pMacAddr[i];
		memset(&(eth_h_mask->et_dest[i]), 0xFF, sizeof(eth_h_mask->et_dest[i]));
	}

	if(bfWPT->packet_type & FLAG_BUFFALO_WITHOUT_UDP_H)
	{
		data = &frame[ETHER_HDR_SIZE];
		data_mask = &mask[ETHER_HDR_SIZE];
		magic_len = 128 - ETHER_HDR_SIZE;
		frame_match_len = ETHER_HDR_SIZE;
	}
	else
	{
		data = &frame[ETHER_HDR_SIZE + IP_HDR_SIZE];
		data_mask = &mask[ETHER_HDR_SIZE + IP_HDR_SIZE];
		ip_h->udp_dst = htons(bfWPT->port_num);
		memset(&(ip_h_mask->udp_dst), 0xFF, sizeof(ip_h_mask->udp_dst));
		magic_len = 128 - ETHER_HDR_SIZE - IP_HDR_SIZE;
		frame_match_len = ETHER_HDR_SIZE + IP_HDR_SIZE;
	}
	if(magic_len > 6 * 17) magic_len = 6*17;
	frame_match_len += magic_len - 1;

	if(debug) DEBUG("matching len = %d\n", magic_len);
	for(i = 0; i < magic_len ; i ++)
	{
		if((i < 6) && (bfWPT->packet_type & FLAG_BUFFALO_ACTIVE_PACKET))
			data[i] = 0xFF;
		else
			data[i] = pMacAddr[i % 6];
		memset(&(data_mask[i]), -1, sizeof(data_mask[i]));
	}

	return frame_match_len;
}

/*******************************************************************************
* bfDumpSramData
*
* DESCRIPTION:
*       Set matching frame pattern and mask information to SRAM of EthPhy.
*
* INPUT:
*       ethPortNum       : No. of target ethernet port.
*       num_frame_pattern: No. of frame pattern to display dump result.
*
* OUTPUT:
*       SRAM dump result.
*
* RETURN:
*       None.
*
*******************************************************************************/
void
bfDumpSramData(MV_U32 ethPortNum, MV_U8 num_frame_pattern)
{
	MV_U8 i;
	MV_U16 old_page;
	MV_U16 reg;
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 22, &old_page);  /* backup present page status */
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,17);  /* change to Page 17 */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 16, &reg);

	DEBUG("verifying SRAM value. pattern=%d\n", num_frame_pattern);
	if(reg & (1 << num_frame_pattern))
		DEBUG("patern %d is Enabled\n", num_frame_pattern);
	else
		DEBUG("patern %d is Disabled\n", num_frame_pattern);

	switch(num_frame_pattern)
	{
	case 0:
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 21, &reg);
		break;
	case 1:
	case 2:
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 20, &reg);
		break;
	case 3:
	case 4:
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 19, &reg);
		break;
	case 5:
	case 6:
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 18, &reg);
		break;
	default:
		return ;
	}
	if(num_frame_pattern == 0 || num_frame_pattern == 1 || num_frame_pattern == 3 || num_frame_pattern == 5)
		DEBUG("Matching frame pattern length = %d Bytes\n", (reg & 0x007F) + 1);
	else if(num_frame_pattern == 2 || num_frame_pattern == 4 || num_frame_pattern == 6)
		DEBUG("Matching frame pattern length = %d Bytes\n", (((reg >> 7) & 0x007F) + 1));

	for(i = 0; i < 128; i++)
	{
		if((i!= 0) && ((i % 0x10) == 0))
			DEBUG("\n0x%02x : ", (i / 0x10) * 0x10);
		else if (((i % 0x10) != 0) && ((i % 0x08) ==0))
			DEBUG(" ");
		else if(i == 0)
			DEBUG("\n0x%02x : ", 0);

		/* read data from SRAM */
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 26, 0x0800 | i | (num_frame_pattern << 7));     /* 0000 0100 0000 0000 | i */

		/* read value recorded in SRAM */
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 28, &reg);
		if(reg & 0x0100)
		{
			DEBUG("%02x ", reg & 0xFF);
		}
		else
		{
			DEBUG("-- ");
		}
	}
	DEBUG("\n");
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, old_page);  /* restore previous page status */
}

/*******************************************************************************
* bfWriteFramePaternToSram
*
* DESCRIPTION:
*       Set matching frame pattern and mask information to SRAM of EthPhy.
*
* INPUT:
*       ethPortNum       : No. of target ethernet port.
*       frame            : 128 byte frame pattern
*       mask             : 128 byte mask information. which frame byte is used to matching.
*       num_frame_pattern: No. of frame pattern to set frame pattern.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
static void
bfWriteFramePaternToSram(MV_U32 ethPortNum, MV_U8 *frame, MV_U8 *mask, MV_U8 num_frame_pattern)
{
	MV_U16 i;
	MV_U16 reg;
	MV_U16 old_page;

	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 22, &old_page);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 17);

	for(i = 0; i < 128; i++)
	{
		/* set value to 17.27 */
		reg = ((mask[i] > 0)? 1:0) << 8 | frame[i];
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 27, reg);
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 26, 0x0400 | i | (num_frame_pattern << 7));     /* 0000 0100 0000 0000 | i */
	}
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, old_page);
}

/*******************************************************************************
* bfWriteWolFrameMatchingLen
*
* DESCRIPTION:
*       Set wol frame matching length to 17.18, 17.19, 17.20 or 17.21
*
* INPUT:
*       ethPortNum       : No. of target ethernet port.
*       num_frame_patern : No. of frame patern to set matching length.
*       matching_len     : matching length to set.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
void
bfWriteWolFrameMatchingLen(MV_U32 ethPortNum, MV_U8 num_frame_pattern, MV_U8 matching_len)
{
	MV_U16 reg;
	MV_U16 old_page;

	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 22, &old_page);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 17);

	switch(num_frame_pattern)
	{
	case 0:
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 21, matching_len);
		break;
	case 1:
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 20, &reg);
		reg &= ~(0x007F);
		reg |= matching_len;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 20, reg);
		break;
	case 2:
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 20, &reg);
		reg &= ~(0x3F80);
		reg |= (matching_len << 7);
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 20, reg);
		break;
	case 3:
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 19, &reg);
		reg &= ~(0x007F);
		reg |= matching_len;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 19, reg);
		break;
	case 4:
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 19, &reg);
		reg &= ~(0x3F80);
		reg |= (matching_len << 7);
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 19, reg);
		break;
	case 5:
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 18, &reg);
		reg &= ~(0x007F);
		reg |= matching_len;
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 18, reg);
		break;
	case 6:
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 18, &reg);
		reg &= ~(0x3F80);
		reg |= (matching_len << 7);
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 18, reg);
		break;
	}
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, old_page);
}

/*******************************************************************************
* bfSetWolFrameWaitMode
*
* DESCRIPTION:
*       Set wol frame wait mode.
*
* INPUT:
*       ethPortNum : No. of target ethernet port.
*       pMacAddr   : MAC address to set.
*       debug      : debug flag(for verbose output)
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE  : success to set wol mode.
*       MV_FALSE : failed to set wol mode.
*
*******************************************************************************/
MV_BOOL
bfSetWolFrameWaitMode(MV_U32 ethPortNum, int debug)
{
	MV_U16 old_page;
	MV_U16 reg;
	MV_U16 enabling_pattern = 0;
	MV_U8 i;
	MV_U8 frame[128];
	MV_U8 mask[128];
	MV_U8 matching_len = 128;
	unsigned char MacAddr[6];

	if(bfIsSupportWol(ethPortNum) == MV_FALSE)
		return MV_FALSE;

#if defined(__KERNEL__) && !defined(MV_UBOOT)
	DEBUG("Setting WOL ... ");
	if(!bfGppInRegBitTest(BIT_PWRAUTO_SW))
	{
		DEBUG("Switch status is not AUTO\n");
		return MV_FALSE;
	}
#endif

	memset(MacAddr, 0, sizeof(MacAddr));
	mvEthMacAddrGet(ethPortNum, MacAddr);

	bfControlWolInterrupt(ethPortNum, FLAG_BUFFALO_WOL_INTERRUPT_DISABLE);

	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 22, &old_page);  /* backup present page status */
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 17);        /* change to Page 17 */
	/* Disabling WOL Match Enable flags */
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 16, &reg);
	reg &= ~(0xC07F);
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 16, reg);

	for(i = 0; i < MAX_WOL_TYPE; i++)
	{
		memset(frame, 0, sizeof(frame));
		memset(mask, 0, sizeof(mask));
		matching_len = bfSetFrameEventWol(frame, mask, MacAddr, &(bfWolPacketType[i]), debug);
		bfWriteFramePaternToSram(ethPortNum, frame, mask, i);
		bfWriteWolFrameMatchingLen(ethPortNum, i, matching_len);
		enabling_pattern |= (1 << i);
	}

	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 16, enabling_pattern);  /* 1001 0000 0000 0011 */

	if(debug)
		for (i = 0; i < 7; i++)
			bfDumpSramData(ethPortNum, i);

	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 16, 0x9000 | enabling_pattern); /* 1001 0000 0000 0011 */

	bfControlWolInterrupt(ethPortNum, FLAG_BUFFALO_WOL_INTERRUPT_ENABLE);
	bfSetPowerSaveMode(ethPortNum);

	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, old_page);  /* restore previous page status */
#if defined(__KERNEL__) && !defined(MV_UBOOT)
	DEBUG("Success\n");
#endif
	return MV_TRUE;
}

/*******************************************************************************
* bfGetWolPatternString
*
* DESCRIPTION:
*       generate string to send kernel from the information of which frame pattern matched.
*
* INPUT:
*       wol_stat : register value of 17.17
*
* OUTPUT:
*       None.
*
* RETURN:
*       pointer to string.
*
*******************************************************************************/
char *bfGetWolPatternString(MV_U16 wol_stat)
{
	MV_U8 i;

	memset(bfWolPatternString, 0, sizeof(bfWolPatternString));
	for(i = 0; i < MAX_WOL_TYPE; i++)
	{
		if(wol_stat & (1 << i) && (strstr(bfWolPatternString, bfWolPacketType[i].name) == 0)) {
			if(strlen(bfWolPatternString) == 0)
				sprintf(bfWolPatternString, "%s", bfWolPacketType[i].name);
			else
				sprintf(bfWolPatternString, ",%s", bfWolPacketType[i].name);
		}
	}

	return (bfWolPatternString[0])? bfWolPatternString : NULL;
}

/*******************************************************************************
* bfGetWolInterruptStatus
*
* DESCRIPTION:
*       Get interrupt status
*
* INPUT:
*       ethPortNum : No. of target ethernet port.
*
* OUTPUT:
*       None.
*
* RETURN:
*       register value of 17.17(which frame pattern received)
*
*******************************************************************************/
MV_U16 bfGetWolInterruptStatus(MV_U32 ethPortNum, int debug)
{
	MV_U16 old_page;
	MV_U16 reg;

	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 22, &old_page);  /* backup present page status */

	if(debug)
	{
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 17);
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 17, &reg);
		DEBUG("WOL status(17.17)=0x%04x\n", reg);
	}

	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 0);
	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 19, &reg);
	if(debug)
	{
		DEBUG("WOL status(0.19)=0x%04x\n", reg);
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 23, &reg);
		DEBUG("Global Interrupt Status(0.23)=0x%04x\n", reg);
	}

	if(reg & 0x0080)
	{
		mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, 17);
		mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 17, &reg);
	}
	else
	{
		reg = 0;
	}

	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, old_page);  /* restore previous page status */

	return (reg & 0x003F);
}

void bfWolInterruptPinDisable(MV_U32 ethPortNum)
{
	MV_U16 reg;
	MV_U16 old_page;

	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum), 22, &old_page);  /* backup present page information */
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),22,3);   /* change to Page 3 */

	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),16, &reg);
	reg &= ~0x0f00;
	reg |= 0x0800;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum),16, reg);

	mvEthPhyRegRead(mvBoardPhyAddrGet(ethPortNum),18, &reg);
	reg &= ~0x0080;
	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 18, reg);

	mvEthPhyRegWrite(mvBoardPhyAddrGet(ethPortNum), 22, old_page);  /* restore previous page */
}

