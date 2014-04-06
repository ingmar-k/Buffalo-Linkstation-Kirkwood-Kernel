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


#ifndef __INCmvBoardEnvSpech
#define __INCmvBoardEnvSpech

#include "mvSysHwConfig.h"


/* For future use */
#define BD_ID_DATA_START_OFFS		0x0
#define BD_DETECT_SEQ_OFFS		0x0
#define BD_SYS_NUM_OFFS			0x4
#define BD_NAME_OFFS			0x8

/* I2C bus addresses */
#define MV_BOARD_CTRL_I2C_ADDR			0x0     /* Controller slave addr */
#define MV_BOARD_CTRL_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_DIMM0_I2C_ADDR			0x56
#define MV_BOARD_DIMM0_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_DIMM1_I2C_ADDR			0x54
#define MV_BOARD_DIMM1_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_EEPROM_I2C_ADDR	    	0x51
#define MV_BOARD_EEPROM_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_MAIN_EEPROM_I2C_ADDR	   	0x50
#define MV_BOARD_MAIN_EEPROM_I2C_ADDR_TYPE 	ADDR7_BIT
#define MV_BOARD_MUX_I2C_ADDR_ENTRY		0x2
#define MV_BOARD_DIMM_I2C_CHANNEL		0x0

#define BOOT_FLASH_INDEX			0
#define MAIN_FLASH_INDEX			1

#define BOARD_ETH_START_PORT_NUM	0

/* Supported clocks */
#define MV_BOARD_TCLK_100MHZ	100000000
#define MV_BOARD_TCLK_125MHZ	125000000
#define MV_BOARD_TCLK_133MHZ	133333333
#define MV_BOARD_TCLK_150MHZ	150000000
#define MV_BOARD_TCLK_166MHZ	166666667
#define MV_BOARD_TCLK_200MHZ	200000000

#define MV_BOARD_SYSCLK_100MHZ	100000000
#define MV_BOARD_SYSCLK_125MHZ	125000000
#define MV_BOARD_SYSCLK_133MHZ	133333333
#define MV_BOARD_SYSCLK_150MHZ	150000000
#define MV_BOARD_SYSCLK_166MHZ	166666667
#define MV_BOARD_SYSCLK_200MHZ	200000000
#define MV_BOARD_SYSCLK_233MHZ	233333333
#define MV_BOARD_SYSCLK_250MHZ	250000000
#define MV_BOARD_SYSCLK_267MHZ	266666667
#define MV_BOARD_SYSCLK_300MHZ	300000000
#define MV_BOARD_SYSCLK_333MHZ	333333334
#define MV_BOARD_SYSCLK_400MHZ	400000000

#define MV_BOARD_REFCLK_25MHZ	 25000000

/* Board specific */
/* =============================== */

/* boards ID numbers */

#define BOARD_ID_BASE           		0x0

/* New board ID numbers */
#define DB_88F6281A_BP_ID			(BOARD_ID_BASE)
#define DB_88F6281_BP_MLL_ID        1680
#define RD_88F6281A_ID				(BOARD_ID_BASE+0x1)
#define RD_88F6281_MLL_ID			1682
#define DB_88F6192A_BP_ID			(BOARD_ID_BASE+0x2)
#define RD_88F6192A_ID				(BOARD_ID_BASE+0x3)
#define RD_88F6192_MLL_ID			1681
#define DB_88F6180A_BP_ID			(BOARD_ID_BASE+0x4)
#define DB_88F6190A_BP_ID			(BOARD_ID_BASE+0x5)
#define RD_88F6190A_ID				(BOARD_ID_BASE+0x6)
#define RD_88F6281A_PCAC_ID			(BOARD_ID_BASE+0x7)
#define DB_CUSTOMER_ID			    (BOARD_ID_BASE+0x8)
#define SHEEVA_PLUG_ID			    (BOARD_ID_BASE+0x9)
#define DB_88F6280A_BP_ID		    (BOARD_ID_BASE+0xA)
#define DB_88F6282A_BP_ID		    (BOARD_ID_BASE+0xB)
#define RD_88F6282A_ID		    		(BOARD_ID_BASE+0xC)
#define MV_MAX_BOARD_ID 			(DB_88F6282A_BP_ID + 1)

/* BUFFALO board ID numbers */
#if defined(CONFIG_BUFFALO_PLATFORM)
 #define BUFFALO_BOARD_ID_BASE		0x80
 #define BF_MVLSXH_ID				(BUFFALO_BOARD_ID_BASE)
 #define BF_MVLSV_ID				(BUFFALO_BOARD_ID_BASE + 0x01)
 #define BF_MVLSWV_ID				(BUFFALO_BOARD_ID_BASE + 0x02)
 #define BF_MVLSQV_ID				(BUFFALO_BOARD_ID_BASE + 0x03)
 #define BF_MVLSXL_ID				(BUFFALO_BOARD_ID_BASE + 0x04)
 #define BF_MVLSXL_GE_ID			(BUFFALO_BOARD_ID_BASE + 0x05)
 #define BF_MVLSXL_GE_V2_ID			(BUFFALO_BOARD_ID_BASE + 0x06)
 #define BF_MVWXL_ID				(BUFFALO_BOARD_ID_BASE + 0x08)
 #define BF_MVWSSX_ID				(BUFFALO_BOARD_ID_BASE + 0x10)
 #define BF_MVTSXEL_ID				(BUFFALO_BOARD_ID_BASE + 0x14)
 #define BF_MVTSXAL_ID				(BUFFALO_BOARD_ID_BASE + 0x18)
 #define BF_MVAVL_ID				(BUFFALO_BOARD_ID_BASE + 0x20)
 #undef MV_MAX_BOARD_ID
 #define MV_MAX_BOARD_ID			(BF_MVAVL_ID + 1)
#endif // defined(CONFIG_BUFFALO_PLATFORM)

/* DB-88F6281A-BP */
#if defined(MV_NAND)
    #define DB_88F6281A_MPP0_7                   	0x21111111
#else
    #define DB_88F6281A_MPP0_7                   	0x21112220
#endif
#define DB_88F6281A_MPP8_15                   	0x11113311
#define DB_88F6281A_MPP16_23                   	0x00551111
#define DB_88F6281A_MPP24_31                   	0x00000000
#define DB_88F6281A_MPP32_39                   	0x00000000
#define DB_88F6281A_MPP40_47                   	0x00000000
#define DB_88F6281A_MPP48_55                   	0x00000000
#define DB_88F6281A_OE_LOW                       0x0
#if defined(MV_TDM_5CHANNELS)
	#define DB_88F6281A_OE_HIGH		(BIT6)
#else
#define DB_88F6281A_OE_HIGH                      0x0
#endif
#define DB_88F6281A_OE_VAL_LOW                   0x0
#define DB_88F6281A_OE_VAL_HIGH                  0x0


/* DB-88F6282A-BP */
#if defined(MV_NAND)
    #define DB_88F6282A_MPP0_7                   	0x21111111
#else
    #define DB_88F6282A_MPP0_7                   	0x21112220
#endif
#define DB_88F6282A_MPP8_15                   	0x11113311
#define DB_88F6282A_MPP16_23                   	0x00551111
#define DB_88F6282A_MPP24_31                   	0x00000000
#define DB_88F6282A_MPP32_39                   	0x00000000
#define DB_88F6282A_MPP40_47                   	0x00000000
#define DB_88F6282A_MPP48_55                   	0x00000000
#define DB_88F6282A_OE_LOW                       0x0
#if defined(MV_TDM_5CHANNELS)
	#define DB_88F6282A_OE_HIGH		(BIT6)
#else
#define DB_88F6282A_OE_HIGH                      0x0
#endif
#define DB_88F6282A_OE_VAL_LOW                   0x0
#define DB_88F6282A_OE_VAL_HIGH                  0x0

/* RD-88F6282 */
#define RD_88F6282A_MPP0_7                   	0x21111111
#define RD_88F6282A_MPP8_15                   	0x433B2211
#define RD_88F6282A_MPP16_23                   	0x33331104
#define RD_88F6282A_MPP24_31                   	0x33023333
#define RD_88F6282A_MPP32_39                   	0x40000033
#define RD_88F6282A_MPP40_47                   	0x22224444
#define RD_88F6282A_MPP48_55                   	0x00000002
#define RD_88F6282A_OE_LOW                       ~(BIT17)
#define RD_88F6282A_OE_HIGH                      ~(BIT2 | BIT3 | BIT4)
#define RD_88F6282A_OE_VAL_LOW                   BIT17
#define RD_88F6282A_OE_VAL_HIGH                  BIT4

/* RD-88F6281A */
#if defined(MV_NAND)
    #define RD_88F6281A_MPP0_7                   	0x21111111
#else
    #define RD_88F6281A_MPP0_7                   	0x21112220
#endif
#define RD_88F6281A_MPP8_15                   	0x11113311
#define RD_88F6281A_MPP16_23                   	0x33331111
#define RD_88F6281A_MPP24_31                   	0x33003333
#define RD_88F6281A_MPP32_39                   	0x20440533
#define RD_88F6281A_MPP40_47                   	0x22202222
#define RD_88F6281A_MPP48_55                   	0x00000002
#define RD_88F6281A_OE_LOW                      (BIT28 | BIT29)
#define RD_88F6281A_OE_HIGH                     (BIT3 | BIT6 | BIT17)
#define RD_88F6281A_OE_VAL_LOW                   0x0
#define RD_88F6281A_OE_VAL_HIGH                  0x0

/* DB-88F6192A-BP */
#if defined(MV_NAND)
    #define DB_88F6192A_MPP0_7                   	0x21111111
#else
    #define DB_88F6192A_MPP0_7                   	0x21112220
#endif
#define DB_88F6192A_MPP8_15                   	0x11113311
#define DB_88F6192A_MPP16_23                   	0x00501111
#define DB_88F6192A_MPP24_31                   	0x00000000
#define DB_88F6192A_MPP32_35                   	0x00000000
#define DB_88F6192A_OE_LOW                       (BIT22 | BIT23)
#define DB_88F6192A_OE_HIGH                      0x0
#define DB_88F6192A_OE_VAL_LOW                   0x0
#define DB_88F6192A_OE_VAL_HIGH                  0x0

/* RD-88F6192A */
#define RD_88F6192A_MPP0_7                   	0x01222222
#define RD_88F6192A_MPP8_15                   	0x00000011
#define RD_88F6192A_MPP16_23                   	0x05550000
#define RD_88F6192A_MPP24_31                   	0x0
#define RD_88F6192A_MPP32_35                   	0x0
#define RD_88F6192A_OE_LOW                      (BIT11 | BIT14 | BIT24 | BIT25 | BIT26 | BIT27 | BIT30 | BIT31)
#define RD_88F6192A_OE_HIGH                     (BIT0 | BIT2)
#define RD_88F6192A_OE_VAL_LOW                  0x18400
#define RD_88F6192A_OE_VAL_HIGH                 0x8

/* DB-88F6180A-BP */
#if defined(MV_NAND)
    #define DB_88F6180A_MPP0_7                   	0x21111111
#else
    #define DB_88F6180A_MPP0_7                   	0x01112222
#endif
#define DB_88F6180A_MPP8_15                   	0x11113311
#define DB_88F6180A_MPP16_23                   	0x00001111
#define DB_88F6180A_MPP24_31                   	0x0
#define DB_88F6180A_MPP32_39                   	0x4444c000
#define DB_88F6180A_MPP40_44                   	0x00044444
#define DB_88F6180A_OE_LOW                       0x0
#define DB_88F6180A_OE_HIGH                      0x0
#define DB_88F6180A_OE_VAL_LOW                   0x0
#define DB_88F6180A_OE_VAL_HIGH                  0x0

/* RD-88F6281A_PCAC */
#define RD_88F6281A_PCAC_MPP0_7                	0x21111111
#define RD_88F6281A_PCAC_MPP8_15               	0x00003311
#define RD_88F6281A_PCAC_MPP16_23              	0x00001100
#define RD_88F6281A_PCAC_MPP24_31              	0x00000000
#define RD_88F6281A_PCAC_MPP32_39              	0x00000000
#define RD_88F6281A_PCAC_MPP40_47              	0x00000000
#define RD_88F6281A_PCAC_MPP48_55              	0x00000000
#define RD_88F6281A_PCAC_OE_LOW                 0x0
#define RD_88F6281A_PCAC_OE_HIGH                0x0
#define RD_88F6281A_PCAC_OE_VAL_LOW             0x0
#define RD_88F6281A_PCAC_OE_VAL_HIGH            0x0

/* DB-88F6280A-BP */
#if defined(MV_NAND)
    #define DB_88F6280A_MPP0_7                 	0x01111111
#else
    #define DB_88F6280A_MPP0_7                 	0x01222222
#endif
#define DB_88F6280A_MPP8_15                   	0x00300011
#define DB_88F6280A_MPP16_23                   	0x00001103
#define DB_88F6280A_MPP24_31                   	0x00000000
#define DB_88F6280A_MPP32_39                   	0x00000000
#define DB_88F6280A_MPP40_47                   	0x00000000
#define DB_88F6280A_MPP48_55                   	0x00000000
#define DB_88F6280A_OE_LOW                      (0xffffffff)
#define DB_88F6280A_OE_HIGH                     0x0
#define DB_88F6280A_OE_VAL_LOW                  0x0
#define DB_88F6280A_OE_VAL_HIGH                 0x0


/* SHEEVA PLUG */
#define RD_SHEEVA_PLUG_MPP0_7                   0x01111111
#define RD_SHEEVA_PLUG_MPP8_15                  0x11113322
#define RD_SHEEVA_PLUG_MPP16_23                 0x00001111
#define RD_SHEEVA_PLUG_MPP24_31                 0x00100000
#define RD_SHEEVA_PLUG_MPP32_39                 0x00000000
#define RD_SHEEVA_PLUG_MPP40_47                 0x00000000
#define RD_SHEEVA_PLUG_MPP48_55                 0x00000000
#define RD_SHEEVA_PLUG_OE_LOW                   0x0
#define RD_SHEEVA_PLUG_OE_HIGH                  0x0
#define RD_SHEEVA_PLUG_OE_VAL_LOW               (BIT29)
#define RD_SHEEVA_PLUG_OE_VAL_HIGH              ((~(BIT17 | BIT16 | BIT15)) | BIT14) 

/* DB-CUSTOMER */
#define DB_CUSTOMER_MPP0_7                	    0x21111111
#define DB_CUSTOMER_MPP8_15               	    0x00003311
#define DB_CUSTOMER_MPP16_23              	    0x00001100
#define DB_CUSTOMER_MPP24_31              	    0x00000000
#define DB_CUSTOMER_MPP32_39              	    0x00000000
#define DB_CUSTOMER_MPP40_47              	    0x00000000
#define DB_CUSTOMER_MPP48_55              	    0x00000000
#define DB_CUSTOMER_OE_LOW                      0x0
#define DB_CUSTOMER_OE_HIGH                     (~((BIT6) | (BIT7) | (BIT8) | (BIT9)))
#define DB_CUSTOMER_OE_VAL_LOW                  0x0
#define DB_CUSTOMER_OE_VAL_HIGH                 0x0

#if defined(CONFIG_BUFFALO_PLATFORM)
#define BUF_NV_ADDR_PID				0x0204
#define BUF_NV_ADDR_SERIES_NAME			0x0210
#define BUF_NV_SIZE_SERIES_NAME			0x0020
#define BUF_NV_ADDR_PRODUCT_NAME		BUF_NV_ADDR_SERIES_NAME + BUF_NV_SIZE_SERIES_NAME // 0x0230
#define BUF_NV_SIZE_PRODUCT_NAME		0x20

/* MVLSXH/MVLSXL */
#define MVLSXH_MPP0_7				0x01222222
#define MVLSXH_MPP8_15				0x11110000
#define MVLSXH_MPP16_23				0x33330011
#define MVLSXH_MPP24_31				0x33003333
#define MVLSXH_MPP32_39				0x00000033
#define MVLSXH_MPP40_47				0x00000000
#define MVLSXH_MPP48_55				0x00000000
#define MVLSXH_OE_LOW			(~((BIT7)|(BIT8)|(BIT10)|(BIT11)|(BIT18)|(BIT19)))
#define MVLSXH_OE_HIGH			(~((BIT4)|(BIT5)|(BIT6)|(BIT7)|(BIT16)))
#define MVLSXH_OE_VAL_LOW		((BIT7)|(BIT8)|(BIT18)|(BIT19))
#define MVLSXH_OE_VAL_HIGH		((BIT4)|(BIT5)|(BIT6)|(BIT7)|(BIT16))
#define MVLSXH_POLARITY_VAL_LOW		((BIT18)|(BIT19))
#define MVLSXH_POLARITY_VAL_HIGH	((BIT4)|(BIT5)|(BIT6)|(BIT7)|(BIT9)|(BIT10)|(BIT11)|(BIT14)|(BIT16)|(BIT17))

/* MVLSXL-GE */
#define MVLSXL_GE_OE_LOW			(~((BIT7)|(BIT8)|(BIT10)))
#define MVLSXL_GE_OE_HIGH			(~((BIT7)))
#define MVLSXL_GE_OE_VAL_LOW			((BIT7)|(BIT8))
#define MVLSXL_GE_OE_VAL_HIGH			((BIT7))
#define MVLSXL_GE_POLARITY_VAL_LOW		(0)
#define MVLSXL_GE_POLARITY_VAL_HIGH		((BIT7)|(BIT9)|(BIT10)|(BIT11))

/* MVLSXL-GE-V2 */
#define MVLSXL_GE_V2_MPP0_7			0x21111111
#define MVLSXL_GE_V2_MPP8_15			0x00003300
#define MVLSXL_GE_V2_MPP16_23			0x00501100
#define MVLSXL_GE_V2_MPP24_31			0x00000000
#define MVLSXL_GE_V2_MPP32_35			0x00000000
#define MVLSXL_GE_V2_OE_LOW			(~((BIT14)|(BIT17)|(BIT26)|(BIT27)))
#define MVLSXL_GE_V2_OE_HIGH			0x0
#define MVLSXL_GE_V2_OE_VAL_LOW			((BIT14)|(BIT17)|(BIT26)|(BIT27))
#define MVLSXL_GE_V2_OE_VAL_HIGH		0x0
#define MVLSXL_GE_V2_POLARITY_VAL_LOW		((BIT17)|(BIT26)|(BIT27))
#define MVLSXL_GE_V2_POLARITY_VAL_HIGH		0x0

/* MVWXL */
#define MVWXL_MPP0_7				0x21111111
#define MVWXL_MPP8_15				0x11113300
#define MVWXL_MPP16_23				0x33331111
#define MVWXL_MPP24_31				0x33003333
#define MVWXL_MPP32_39				0x00000033
#define MVWXL_MPP40_47				0x00000000
#define MVWXL_MPP48_55				0x00000000
#define MVWXL_OE_LOW				(~((BIT8)|(BIT28)|(BIT29)))
#define MVWXL_OE_HIGH				(~((BIT2)|(BIT4)|(BIT5)|(BIT6)|(BIT7)|(BIT14)|(BIT15)|(BIT16)|(BIT17)))
#define MVWXL_OE_VAL_LOW			(0)
#define MVWXL_OE_VAL_HIGH			((BIT4)|(BIT6)|(BIT15)|(BIT16)|(BIT17))
#define MVWXL_POLARITY_VAL_LOW			(0)
#define MVWXL_POLARITY_VAL_HIGH			((BIT4)|(BIT6)|(BIT9)|(BIT10)|(BIT11)|(BIT15)|(BIT16)|(BIT17))

/* MVWSSX */
#define MVWSSX_MPP0_7				0x21111111
#define MVWSSX_MPP8_15				0x11113300
#define MVWSSX_MPP16_23				0x33331111
#define MVWSSX_MPP24_31				0x33003333
#define MVWSSX_MPP32_39				0x00000033
#define MVWSSX_MPP40_47				0x00000000
#define MVWSSX_MPP48_55				0x00000000
#define MVWSSX_OE_LOW			(~((BIT28)))
#define MVWSSX_OE_HIGH			(~((BIT2)|(BIT3)|(BIT4)|(BIT5)|(BIT6)|(BIT7)|(BIT14)|(BIT17)))
#define MVWSSX_OE_VAL_LOW		0
#define MVWSSX_OE_VAL_HIGH		((BIT4)|(BIT6)|(BIT17))
#define MVWSSX_POLARITY_VAL_LOW		0
#define MVWSSX_POLARITY_VAL_HIGH	((BIT4)|(BIT6)|(BIT9)|(BIT10)|(BIT11)|(BIT15)|(BIT16)|(BIT17))

/* MVTSXEL */
#define MVTSXEL_MPP0_7				0x21111111
#define MVTSXEL_MPP8_15				0x03303300
#define MVTSXEL_MPP16_23			0x33331111
#define MVTSXEL_MPP24_31			0x33003333
#define MVTSXEL_MPP32_39			0x00000033
#define MVTSXEL_MPP40_47			0x00000000
#define MVTSXEL_MPP48_55			0x00000000
#define MVTSXEL_OE_LOW			(~((BIT8)|(BIT28)|(BIT29)))
#define MVTSXEL_OE_HIGH			(~((BIT2)|(BIT4)|(BIT5)|(BIT6)|(BIT7)|(BIT14)|(BIT15)|(BIT16)|(BIT17)))
#define MVTSXEL_OE_VAL_LOW		0
#define MVTSXEL_OE_VAL_HIGH		((BIT4)|(BIT6)|(BIT15)|(BIT16)|(BIT17))
#define MVTSXEL_POLARITY_VAL_LOW	0
#define MVTSXEL_POLARITY_VAL_HIGH	((BIT5)|(BIT6)|(BIT12))

/* MVAVL */
#define MVAVL_MPP0_7				0x01222222
#define MVAVL_MPP8_15				0x10310010
#define MVAVL_MPP16_23				0x00000011
#define MVAVL_MPP24_31				0x00000000
#define MVAVL_MPP32_39				0x00000000
#define MVAVL_MPP40_47				0x00000000
#define MVAVL_MPP48_55				0x00000000
#define MVAVL_OE_LOW				(~((BIT31)|(BIT30)|(BIT29)|(BIT28)|(BIT27)|(BIT26)|(BIT25)|(BIT24)|\
						   (BIT23)|(BIT22)|(BIT21)|(BIT20)|(BIT19)|(BIT18)|(BIT14)|(BIT11)|(BIT10)|(BIT1)))/* BIT1 FE-MISC-70 */
#define MVAVL_OE_HIGH				(~((BIT0)|(BIT1)|(BIT2)|(BIT3)|(BIT5)|(BIT6)|(BIT7)|(BIT15)|(BIT16)))
#define MVAVL_OE_VAL_LOW			((BIT14)|(BIT18)|(BIT19))
#define MVAVL_OE_VAL_HIGH			((BIT2)|(BIT3)|(BIT5)|(BIT6)|(BIT7)|(BIT16))
#define MVAVL_POLARITY_VAL_LOW			((BIT14)|(BIT18)|(BIT19))
#define MVAVL_POLARITY_VAL_HIGH			((BIT2)|(BIT3)|(BIT5)|(BIT6)|(BIT7)|(BIT9)|(BIT11)|(BIT15)|(BIT16)|(BIT17))

/* MVLSV */
#define MVLSV_MPP0_7				0x21111111
#define MVLSV_MPP8_15				0x00003300
#define MVLSV_MPP16_23				0x00001100
#define MVLSV_MPP24_31				0x00000000
#define MVLSV_MPP32_39				0x00000000
#define MVLSV_MPP40_47				0x00000000
#define MVLSV_MPP48_55				0x00000000
#define MVLSV_OE_LOW				(~((BIT8)|(BIT9)|(BIT12)|(BIT16)|(BIT17)))
#define MVLSV_OE_HIGH				(~((BIT4)|(BIT5)|(BIT6)|(BIT7)|(BIT8)))
#define MVLSV_OE_VAL_LOW			((BIT16)|(BIT17))
#define MVLSV_OE_VAL_HIGH			((BIT8))
#define MVLSV_POLARITY_VAL_LOW			((BIT16)|(BIT17))
#define MVLSV_POLARITY_VAL_HIGH			((BIT8)|(BIT12)|(BIT13)|(BIT14)|(BIT15)|(BIT16))

/* MVLSWV */
#define MVLSWV_MPP0_7				0x21111111
#define MVLSWV_MPP8_15				0x00003300
#define MVLSWV_MPP16_23				0x00001100
#define MVLSWV_MPP24_31				0x00000000
#define MVLSWV_MPP32_39				0x00000000
#define MVLSWV_MPP40_47				0x00000000
#define MVLSWV_MPP48_55				0x00000000
#define MVLSWV_OE_LOW				(~((BIT8)|(BIT9)|(BIT12)|(BIT16)|(BIT17)))
#define MVLSWV_OE_HIGH				(~((BIT2)|(BIT3)|(BIT4)|(BIT5)|(BIT6)|(BIT7)|(BIT8)))
#define MVLSWV_OE_VAL_LOW			((BIT16)|(BIT17))
#define MVLSWV_OE_VAL_HIGH			((BIT8))
#define MVLSWV_POLARITY_VAL_LOW			((BIT16)|(BIT17))
#define MVLSWV_POLARITY_VAL_HIGH		((BIT8)|(BIT12)|(BIT13)|(BIT14)|(BIT15)|(BIT16))

/* MVLSQV */
#define MVLSQV_MPP0_7				0x21111111
#define MVLSQV_MPP8_15				0x00003300
#define MVLSQV_MPP16_23				0x00001100
#define MVLSQV_MPP24_31				0x00000000
#define MVLSQV_MPP32_39				0x00000000
#define MVLSQV_MPP40_47				0x00000000
#define MVLSQV_MPP48_55				0x00000000
#define MVLSQV_OE_LOW				(~((BIT8)|(BIT9)|(BIT12)|(BIT16)|(BIT17)|(BIT22)|(BIT23)|(BIT24)|(BIT25)))
#define MVLSQV_OE_HIGH				(~((BIT2)|(BIT3)|(BIT4)|(BIT5)|(BIT6)|(BIT7)|(BIT8)))
#define MVLSQV_OE_VAL_LOW			((BIT16)|(BIT17)|(BIT24)|(BIT25))
#define MVLSQV_OE_VAL_HIGH			((BIT2)|(BIT3)|(BIT8))
#define MVLSQV_POLARITY_VAL_LOW			((BIT16)|(BIT17)|(BIT24)|(BIT25))
#define MVLSQV_POLARITY_VAL_HIGH		((BIT2)|(BIT3)|(BIT8)|(BIT12)|(BIT13)|(BIT14)|(BIT16))

#endif // CONFIG_BUFFALO_PLATFORM

#endif /* __INCmvBoardEnvSpech */
