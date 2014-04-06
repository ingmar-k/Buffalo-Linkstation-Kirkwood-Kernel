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
#ifndef __INCmvTdmh
#define __INCmvTdmh

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/mvCtrlEnvAddrDec.h"
#include "voiceband/tdm/mvTdmRegs.h"
#include "ctrlEnv/sys/mvSysTdm.h"
#include "voiceband/voiceband.h"

/* Defines */
#define SAMPLES_BUFF_SIZE(bandMode, factor)  \
	 ((bandMode == MV_NARROW_BAND) ? (factor * 80) : (factor * 160))

#define MV_TDM_CH_BUFF_SIZE(pcmFormat, bandMode, factor) \
	(pcmFormat == MV_PCM_FORMAT_LINEAR ? (2 * SAMPLES_BUFF_SIZE(bandMode, factor)) : \
						  SAMPLES_BUFF_SIZE(bandMode, factor))

#define MV_TDM_AGGR_BUFF_SIZE(pcmFormat, bandMode, factor)	(2 * MV_TDM_CH_BUFF_SIZE(pcmFormat, bandMode, factor))
#define MV_TDM_TOTAL_CHANNELS					2
#define MV_TDM_2CHANNELS_SUPPORT				2
#define MV_TDM_INT_COUNTER					2
#define MV_TDM_MAX_SAMPLE_PERIOD				30 /* ms */
#define MV_TDM_BASE_SAMPLE_PERIOD				10 /* ms */

/* TDM IRQ types */
#define MV_EMPTY_INT		0
#define MV_RX_INT 		BIT0
#define	MV_TX_INT 		BIT1
#define	MV_PHONE_INT 		BIT2 
#define	MV_ERROR_INT 		BIT3

/* PCM SLOT configuration */
/* Assume Rx & Tx have the same slots always */
#define PCM_SLOT_PCLK	8
#define CH0_RX_SLOT	2
#define CH0_TX_SLOT	2
#define CH1_RX_SLOT	10
#define CH1_TX_SLOT	10

/* WB Mode */
#define CH0_RX_DELAY	(2 * PCM_SLOT_PCLK)
#define CH0_TX_DELAY	(0 * PCM_SLOT_PCLK)
#define CH1_RX_DELAY	(14 * PCM_SLOT_PCLK)
#define CH1_TX_DELAY	(14 * PCM_SLOT_PCLK)

#define CH0_WB_RX_DELAY	(10 * PCM_SLOT_PCLK)
#define CH0_WB_TX_DELAY	(10 * PCM_SLOT_PCLK)
#define CH1_WB_RX_DELAY	(18 * PCM_SLOT_PCLK)
#define CH1_WB_TX_DELAY	(18 * PCM_SLOT_PCLK)

#define TDM_INT_SLIC	(DMA_ABORT_BIT|SLIC_INT_BIT)
#define TDM_INT_TX(ch)	(TX_UNDERFLOW_BIT(ch)|TX_BIT(ch)|TX_IDLE_BIT(ch))
#define TDM_INT_RX(ch)	(RX_OVERFLOW_BIT(ch)|RX_BIT(ch)|RX_IDLE_BIT(ch))

/* TDM Registers Configuration */
#if defined(MV_TDM_USE_EXTERNAL_PCLK_SOURCE)
#define CONFIG_PCM_CRTL (MASTER_PCLK_EXTERNAL | MASTER_FS_TDM |DATA_POLAR_NEG | \
			 FS_POLAR_NEG | INVERT_FS_HI | FS_TYPE_SHORT	 | \
			 CH_DELAY_DISABLE 		 		 | \
			 CH_QUALITY_DISABLE | QUALITY_POLARITY_NEG	 | \
			 QUALITY_TYPE_TIME_SLOT | CS_CTRL_DONT_CARE 	 | \
			 WIDEBAND_OFF | PERF_GBUS_TWO_ACCESS)

#else
#define CONFIG_PCM_CRTL (MASTER_PCLK_TDM | MASTER_FS_TDM |DATA_POLAR_NEG | \
			 FS_POLAR_NEG | INVERT_FS_HI | FS_TYPE_SHORT	 | \
			 CH_DELAY_DISABLE 				 | \
			 CH_QUALITY_DISABLE | QUALITY_POLARITY_NEG	 | \
			 QUALITY_TYPE_TIME_SLOT | CS_CTRL_DONT_CARE 	 | \
			 WIDEBAND_OFF | PERF_GBUS_TWO_ACCESS)
#endif

#if defined(MV_TDM_USE_EXTERNAL_PCLK_SOURCE)
#define CONFIG_WB_PCM_CRTL (MASTER_PCLK_EXTERNAL | MASTER_FS_TDM |DATA_POLAR_NEG | \
			    FS_POLAR_NEG | INVERT_FS_HI | FS_TYPE_SHORT	 | \
			    CH_DELAY_ENABLE 				 | \
			    CH_QUALITY_DISABLE | QUALITY_POLARITY_NEG	 | \
			    QUALITY_TYPE_TIME_SLOT | CS_CTRL_DONT_CARE 	 | \
			    WIDEBAND_ON | PERF_GBUS_TWO_ACCESS)
#else
#define CONFIG_WB_PCM_CRTL (MASTER_PCLK_TDM | MASTER_FS_TDM |DATA_POLAR_NEG | \
			    FS_POLAR_NEG | INVERT_FS_HI | FS_TYPE_SHORT	 | \
			    CH_DELAY_ENABLE 				 | \
			    CH_QUALITY_DISABLE | QUALITY_POLARITY_NEG	 | \
			    QUALITY_TYPE_TIME_SLOT | CS_CTRL_DONT_CARE 	 | \
			    WIDEBAND_ON | PERF_GBUS_TWO_ACCESS)
#endif

#define CONFIG_TIMESLOT_CTRL ((CH0_RX_SLOT<<CH0_RX_SLOT_OFFS) | \
			      (CH0_TX_SLOT<<CH0_TX_SLOT_OFFS) | \
			      (CH1_RX_SLOT<<CH1_RX_SLOT_OFFS) | \
			      (CH1_TX_SLOT<<CH1_TX_SLOT_OFFS))

#define CONFIG_CH_SAMPLE(bandMode,factor) ((SAMPLES_BUFF_SIZE(bandMode, factor)<<TOTAL_CNT_OFFS) |\
									 (INT_SAMPLE<<INT_CNT_OFFS))

#define CONFIG_CH0_DELAY_CTRL_CONFIG	((CH0_RX_DELAY << CH_RX_DELAY_OFFS) | \
					 (CH0_TX_DELAY << CH_TX_DELAY_OFFS))

#define CONFIG_CH1_DELAY_CTRL_CONFIG	((CH1_RX_DELAY << CH_RX_DELAY_OFFS) | \
					 (CH1_TX_DELAY << CH_TX_DELAY_OFFS))

#define CONFIG_CH0_WB_DELAY_CTRL_CONFIG	((CH0_WB_RX_DELAY << CH_RX_DELAY_OFFS) | \
					 (CH0_WB_TX_DELAY << CH_TX_DELAY_OFFS))

#define CONFIG_CH1_WB_DELAY_CTRL_CONFIG	((CH1_WB_RX_DELAY << CH_RX_DELAY_OFFS) | \
					 (CH1_WB_TX_DELAY << CH_TX_DELAY_OFFS))
/* Enumerators */	

/* Structures */
typedef struct
{
	MV_U8* tdmRxBuff;
	MV_U8* tdmTxBuff;
	MV_U32 intType;
} mv_tdm_int_info_t;

typedef struct
{
	mv_band_mode_t bandMode;
	mv_pcm_format_t	pcmFormat;
	unsigned char samplePeriod;
} mv_tdm_params_t;


/* APIs */
MV_STATUS mvTdmInit(mv_tdm_params_t* tdmParams);
MV_VOID mvTdmRelease(MV_VOID);
MV_VOID mvTdmIntLow(mv_tdm_int_info_t* tdmIntInfo);
MV_VOID mvTdmPcmStart(MV_VOID);
MV_VOID mvTdmPcmStop(MV_VOID);
MV_STATUS mvTdmTx(MV_U8* tdmTxBuff);
MV_STATUS mvTdmRx(MV_U8* tdmRxBuff);
MV_VOID mvTdmRegsDump(MV_VOID);
MV_STATUS mvTdmSpiRead(MV_U32 val1, MV_U32 val2, MV_U32 cmd, MV_U8 cs, MV_U8 *data);
MV_STATUS mvTdmSpiWrite(MV_U32 val1, MV_U32 val2, MV_U32 cmd, MV_U8 cs);
MV_U8 currRxSampleGet(MV_U8 ch);
MV_U8 currTxSampleGet(MV_U8 ch);

#endif /* __INCmvTdmh */
