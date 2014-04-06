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


#ifndef __INCBuffaloBoard5182EnvSpech
#define __INCBuffaloBoard5182EnvSpech


/* Customer based boards ID numbers */
/* =============================== */

#if defined(CONFIG_BUFFALO_PLATFORM)

#define BUFFALO_BOARD_DEFAULT_MPP0_7		0x00000003
#define BUFFALO_BOARD_DEFAULT_MPP8_15		0x00000000
#define BUFFALO_BOARD_DEFAULT_MPP16_23		0x00005555
#define BUFFALO_BOARD_DEFAULT_MPP_DEV		0x03FF0000
#define BUFFALO_BOARD_DEFAULT_GPP_OE		0xFFC03DF0
#define BUFFALO_BOARD_DEFAULT_GPP_VAL		0x00000000
#define BUFFALO_BOARD_DEFAULT_GPP_POLVAL	0x004004C0

#define BOARD_ID_BUFFALO_BASE			0xF0
#define BOARD_ID_BUFFALO_MAX			(BOARD_ID_BUFFALO_BASE+0x1)

#define BOARD_ID_BASE				BOARD_ID_BUFFALO_BASE
#define MV_MAX_BOARD_ID				BOARD_ID_BUFFALO_MAX

#define CFG_ENV_B_SIZE				0x0400

#define DB_BUF_BOARD_MAX_MPP_CONFIG_NUM		0x1
#define DB_BUF_BOARD_MAX_DEVICE_CONFIG_NUM	0x4
#define DB_BUF_BOARD_MAX_PCI_IF_NUM		0x4
#define DB_BUF_BOARD_MAX_TWSI_DEF_NUM		0x4
#define DB_BUF_BOARD_MAX_MAC_INFO_NUM		0x4
#define DB_BUF_BOARD_MAX_GPP_INFO_NUM		0x4
#define DB_BUF_BOARD_MAX_DEBUG_LED_NUM		0x4

#define DB_BUF_BOARD_DEFAULT_MPP_CONFIG_NUM	0x1
#define DB_BUF_BOARD_DEFAULT_DEVICE_CONFIG_NUM	0x2
#define DB_BUF_BOARD_DEFAULT_PCI_IF_NUM		0x0
#define DB_BUF_BOARD_DEFAULT_TWSI_DEF_NUM	0x1
#define DB_BUF_BOARD_DEFAULT_MAC_INFO_NUM	0x1
#define DB_BUF_BOARD_DEFAULT_GPP_INFO_NUM	0x1
#define DB_BUF_BOARD_DEFAULT_DEBUG_LED_NUM	0x0

#define BUF_NV_ADDR_VERSION			0x0000
#define BUF_NV_ADDR_INTS_GPP_MASK		0x0008
#define BUF_NV_ADDR_NUM_BOARD_MPP		0x000C
#define BUF_NV_ADDR_NUM_BOARD_DEVICE_IF		0x000D
#define BUF_NV_ADDR_NUM_BOARD_PCI_IF		0x000E
#define BUF_NV_ADDR_NUM_BOARD_TWSI_DEV		0x000F
#define BUF_NV_ADDR_NUM_BOARD_MAC_INFO		0x0010
#define BUF_NV_ADDR_NUM_BOARD_GPP_INFO		0x0011
#define BUF_NV_ADDR_ACTIVE_LEDS_NUMBER		0x0012
#define BUF_NV_ADDR_LEDS_POLARITY		0x0013
#define BUF_NV_ADDR_GPP_OUT_EN_VAL		0x0014
#define BUF_NV_ADDR_GPP_OUT_VAL			0x0018
#define BUF_NV_ADDR_GPP_POLARITY_VAL		0x001C

#define BUF_NV_ADDR_BOARD_MPP_INFO		0x0040

#define BUF_NV_ADDR_DEV_CS_INFO			0x0070
#define BUF_NV_ADDR_DEV_CS_INFO_0		0x0070
#define BUF_NV_ADDR_DEV_CS_INFO_1		0x0078
#define BUF_NV_ADDR_DEV_CS_INFO_2		0x0080
#define BUF_NV_ADDR_DEV_CS_INFO_3		0x0088

#define BUF_NV_ADDR_BOARD_PCI_INFO		0x0090
#define BUF_NV_ADDR_BOARD_PCI_INFO_0		0x0090
#define BUF_NV_ADDR_BOARD_PCI_INFO_1		0x0098
#define BUF_NV_ADDR_BOARD_PCI_INFO_2		0x00A0
#define BUF_NV_ADDR_BOARD_PCI_INFO_3		0x00A8

#define BUF_NV_ADDR_BOARD_TWSI_INFO		0x00B0
#define BUF_NV_ADDR_BOARD_MAC_INFO		0x00C0
#define BUF_NV_ADDR_BOARD_GPP_INFO		0x00C8
#define BUF_NV_ADDR_LED_GPP_PIN			0x00D0

#define BUF_NV_ADDR_BIT_INPUT			0x00E0
#define BUF_NV_SIZE_BIT_INPUT			0x20
#define BUF_NV_OFFS_BIT_MICON			0x0
#define BUF_NV_OFFS_BIT_RTC			0x1
#define BUF_NV_OFFS_BIT_UPS			0x2
#define BUF_NV_OFFS_BIT_OMR_BL			0x3
#define BUF_NV_OFFS_BIT_FAN_LOCK		0x4
#define BUF_NV_OFFS_SW_POWER			0x5
#define BUF_NV_OFFS_SW_INIT			0x6
#define BUF_NV_OFFS_SW_RAID			0x7
#define BUF_NV_OFFS_SW_AUTO_POWER		0x8
#define BUF_NV_OFFS_SW_FUNC			0x9
#define BUF_NV_ADDR_BIT_MICON			BUF_NV_ADDR_BIT_INPUT + BUF_NV_OFFS_BIT_MICON
#define BUF_NV_ADDR_BIT_RTC			BUF_NV_ADDR_BIT_INPUT + BUF_NV_OFFS_BIT_RTC
#define BUF_NV_ADDR_BIT_UPS			BUF_NV_ADDR_BIT_INPUT + BUF_NV_OFFS_BIT_UPS
#define BUF_NV_ADDR_BIT_OMR_BL			BUF_NV_ADDR_BIT_INPUT + BUF_NV_OFFS_BIT_OMR_BL
#define BUF_NV_ADDR_BIT_FAN_LOCK		BUF_NV_ADDR_BIT_INPUT + BUF_NV_OFFS_BIT_FAN_LOCK
#define BUF_NV_ADDR_SW_POWER			BUF_NV_ADDR_BIT_INPUT + BUF_NV_OFFS_SW_POWER
#define BUF_NV_ADDR_SW_INIT			BUF_NV_ADDR_BIT_INPUT + BUF_NV_OFFS_SW_INIT
#define BUF_NV_ADDR_SW_RAID			BUF_NV_ADDR_BIT_INPUT + BUF_NV_OFFS_SW_RAID
#define BUF_NV_ADDR_SW_AUTO_POWER		BUF_NV_ADDR_BIT_INPUT + BUF_NV_OFFS_SW_AUTO_POWER
#define BUF_NV_ADDR_SW_FUNC			BUF_NV_ADDR_BIT_INPUT + BUF_NV_OFFS_SW_FUNC

#define BUF_NV_ADDR_BIT_HDD_POWER		BUF_NV_ADDR_BIT_INPUT + BUF_NV_SIZE_BIT_INPUT // 0x0100
#define BUF_NV_SIZE_BIT_HDD_POWER		0x10

#define BUF_NV_ADDR_BIT_USB_POWER		BUF_NV_ADDR_BIT_HDD_POWER + BUF_NV_SIZE_BIT_HDD_POWER // 0x0110
#define BUF_NV_SIZE_BIT_USB_POWER		0x04

#define BUF_NV_ADDR_BIT_OUTPUT			BUF_NV_ADDR_BIT_USB_POWER + BUF_NV_SIZE_BIT_USB_POWER // 0x0114
#define BUF_NV_SIZE_BIT_OUTPUT			0xC
#define BUF_NV_OFFS_BIT_FAN_LOW			0x0
#define BUF_NV_OFFS_BIT_FAN_HIGH		0x1
#define BUF_NV_OFFS_BIT_FAN_FULL		0x2
#define BUF_NV_OFFS_BIT_FAN_STOP		0x3
#define BUF_NV_ADDR_BIT_FAN_LOW			BUF_NV_ADDR_BIT_OUTPUT + BUF_NV_OFFS_BIT_FAN_LOW
#define BUF_NV_ADDR_BIT_FAN_HIGH		BUF_NV_ADDR_BIT_OUTPUT + BUF_NV_OFFS_BIT_FAN_HIGH
#define BUF_NV_ADDR_BIT_FAN_FULL		BUF_NV_ADDR_BIT_OUTPUT + BUF_NV_OFFS_BIT_FAN_FULL
#define BUF_NV_ADDR_BIT_FAN_STOP		BUF_NV_ADDR_BIT_OUTPUT + BUF_NV_OFFS_BIT_FAN_STOP

#define BUF_NV_ADDR_BIT_LED			BUF_NV_ADDR_BIT_OUTPUT + BUF_NV_SIZE_BIT_OUTPUT // 0x0120
#define BUF_NV_SIZE_BIT_LED			0x10
#define BUF_NV_OFFS_BIT_LED_POWER		0x0
#define BUF_NV_OFFS_BIT_LED_INFO		0x1
#define BUF_NV_OFFS_BIT_LED_ALARM		0x2
#define BUF_NV_OFFS_BIT_LED_FUNC		0x3
#define BUF_NV_OFFS_BIT_LED_ETH			0x4
#define BUF_NV_ADDR_BIT_LED_POWER		BUF_NV_ADDR_BIT_LED + BUF_NV_OFFS_BIT_LED_POWER
#define BUF_NV_ADDR_BIT_LED_INFO		BUF_NV_ADDR_BIT_LED + BUF_NV_OFFS_BIT_LED_INFO
#define BUF_NV_ADDR_BIT_LED_ALARM		BUF_NV_ADDR_BIT_LED + BUF_NV_OFFS_BIT_LED_ALARM
#define BUF_NV_ADDR_BIT_LED_FUNC		BUF_NV_ADDR_BIT_LED + BUF_NV_OFFS_BIT_LED_FUNC
#define BUF_NV_ADDR_BIT_LED_ETH			BUF_NV_ADDR_BIT_LED + BUF_NV_OFFS_BIT_LED_ETH

#define BUF_NV_ADDR_LED_GRP			BUF_NV_ADDR_BIT_LED + BUF_NV_SIZE_BIT_LED // 0x0130
#define BUF_NV_ADDR_LED_GRP_POWER		BUF_NV_ADDR_LED_GRP + BUF_NV_OFFS_BIT_LED_POWER
#define BUF_NV_ADDR_LED_GRP_INFO		BUF_NV_ADDR_LED_GRP + BUF_NV_OFFS_BIT_LED_INFO
#define BUF_NV_ADDR_LED_GRP_ALARM		BUF_NV_ADDR_LED_GRP + BUF_NV_OFFS_BIT_LED_ALARM
#define BUF_NV_ADDR_LED_GRP_FUNC		BUF_NV_ADDR_LED_GRP + BUF_NV_OFFS_BIT_LED_FUNC
#define BUF_NV_ADDR_LED_GRP_ETH			BUF_NV_ADDR_LED_GRP + BUF_NV_OFFS_BIT_LED_ETH

#define BUF_NV_ADDR_USE_SWITCH_SLIDE_POWER	0x0140

#define BUF_NV_ADDR_PID				0x0204
#define BUF_NV_ADDR_SERIES_NAME			0x0210
#define BUF_NV_SIZE_SERIES_NAME			0x0020

#define BUF_NV_ADDR_PRODUCT_NAME		BUF_NV_ADDR_SERIES_NAME + BUF_NV_SIZE_SERIES_NAME // 0x0230
#define BUF_NV_SIZE_PRODUCT_NAME		0x20

#define MVTSQ_BOARD_GPP_INFO_NUM		12
#endif // end of CONFIG_BUFFALO_PLATFORM

/*
 * Move from mach-feroceon-mv78xx0/mv78xx0_family/boardEnv/mvBoardEnvLib.h.
 * This enumeration obsolated linux-feroceon_3_1_3_mv78x_mv88f6323.
 */
typedef enum _devGppBoardClass
{
	BOARD_GPP_RTC,
	BOARD_GPP_MV_SWITCH,
	BOARD_GPP_USB_VBUS,
	BOARD_GPP_USB_VBUS_EN,
	BOARD_GPP_USB_OC,
	BOARD_GPP_USB_HOST_DEVICE,
	BOARD_GPP_REF_CLCK,
	BOARD_GPP_VOIP_SLIC,
	BOARD_GPP_LIFELINE,
	BOARD_GPP_BUTTON,
	BOARD_GPP_TS_BUTTON_C,
	BOARD_GPP_TS_BUTTON_U,
	BOARD_GPP_TS_BUTTON_D,
	BOARD_GPP_TS_BUTTON_L,
	BOARD_GPP_TS_BUTTON_R,
	BOARD_GPP_POWER_BUTTON,
	BOARD_GPP_RESTOR_BUTTON,
	BOARD_GPP_WPS_BUTTON,
	BOARD_GPP_HDD0_POWER,
	BOARD_GPP_HDD1_POWER,
	BOARD_GPP_FAN_POWER,
	BOARD_GPP_RESET,
	BOARD_GPP_POWER_ON_LED,
	BOARD_GPP_HDD_POWER,
	BOARD_GPP_SDIO_POWER,
	BOARD_GPP_SDIO_DETECT,
	BOARD_GPP_SDIO_WP,
	BOARD_GPP_SWITCH_PHY_INT,
	BOARD_GPP_TSU_DIRCTION,
	BOARD_GPP_OTHER,
#if defined(CONFIG_BUFFALO_PLATFORM)
	BOARD_GPP_BOARD_ID,
	BOARD_GPP_FAN_LOW,
	BOARD_GPP_FAN_HIGH,
	BOARD_GPP_FAN_LOCK,
	BOARD_GPP_FUNC_LED,
	BOARD_GPP_ALARM_LED,
	BOARD_GPP_INFO_LED,
	BOARD_GPP_PWR_LED,
	BOARD_GPP_FUNC_SW,
	BOARD_GPP_PWR_SW,
	BOARD_GPP_PWRAUTO_SW,
	BOARD_GPP_INIT_SW,
	BOARD_GPP_SATA_HOT,
	BOARD_GPP_MC_IRQ,
	BOARD_GPP_FAN_SELECT,
#endif
} MV_BOARD_GPP_CLASS;

#include "mvTypes.h"
typedef struct _boardGppInfo
{
	MV_BOARD_GPP_CLASS      devClass;
	MV_U8   gppPinNum;

} MV_BOARD_GPP_INFO;

#endif /* __INCmvBoardCustomerEnvSpech */
