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
 
 
/* includes */
#include "rtc/integ_rtc/mvRtc.h"
#include "rtc/integ_rtc/mvRtcReg.h"

#if defined(CONFIG_BUFFALO_PLATFORM)
  // for MagicKeys
  #include "buffalo/kernevnt.h"
#endif

/*******************************************************************************
* mvRtcTimeSet - Update the Real Time Clock.
*
* DESCRIPTION:
*       This function sets a new time and date to the RTC from the given 
*       structure RTC_TIME . All fields within the structure must be assigned 
*		with a value prior to the use of this function.
*
* INPUT:
*       time - A pointer to a structure RTC_TIME (defined in mv.h).
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvRtcTimeSet(MV_RTC_TIME* mvTime)
{
	MV_U32 timeVal = 0;
	MV_U32 dateVal = 0;
	MV_U32 tens;
	MV_U32 single;
#if defined(CONFIG_BUFFALO_PLATFORM)
	MV_U32 BuffaloFlag = 0;
#endif
	
	/* seconds */
	tens = mvTime->seconds / 10;
	single = mvTime->seconds % 10;
	timeVal |= ((tens << RTC_TIME_10SECONDS_SHF) & RTC_TIME_10SECONDS_MSK) |
		  (( single << RTC_TIME_SECONDS_SHF) & RTC_TIME_SECONDS_MSK);

	/* minutes */
	tens = mvTime->minutes / 10;
	single = mvTime->minutes % 10;
	timeVal |= ((tens  << RTC_TIME_10MINUTES_SHF) & RTC_TIME_10MINUTES_MSK) |
		  (( single << RTC_TIME_MINUTES_SHF) & RTC_TIME_MINUTES_MSK);

	/* hours (24) */
	tens = mvTime->hours / 10;
	single = mvTime->hours % 10;
	timeVal |= ((tens  << RTC_TIME_10HOUR_SHF) & RTC_TIME_10HOUR_MSK) |
		  (( single  << RTC_TIME_HOUR_SHF) & RTC_TIME_HOUR_MSK);
  
	/* day */
	single = ++(mvTime->day);
	timeVal |= ((single << RTC_TIME_DAY_SHF ) & RTC_TIME_DAY_MSK);

	/* Update RTC Time Register */
	MV_REG_WRITE(RTC_TIME_REG, timeVal);

#if defined(CONFIG_BUFFALO_PLATFORM)
	/* read Date register */
	BuffaloFlag = ((MV_REG_READ(RTC_DATE_REG) & RTC_DATE_BUFFALO_FLAG_MSK) >> RTC_DATE_BUFFALO_FLAG_SHF);
#endif

	/* date */
	tens = mvTime->date / 10;
	single = mvTime->date % 10;
	dateVal = ((tens  << RTC_DATE_10DAY_SHF) & RTC_DATE_10DAY_MSK) |
		  (( single << RTC_DATE_DAY_SHF) & RTC_DATE_DAY_MSK);

	/* month */
	tens = mvTime->month / 10;
	single = mvTime->month % 10;
	dateVal |= ((tens  << RTC_DATE_10MONTH_SHF) & RTC_DATE_10MONTH_MSK) |
		  (( single << RTC_DATE_MONTH_SHF) & RTC_DATE_MONTH_MSK);
    
	/* year */
	tens = mvTime->year / 10;
	single = mvTime->year % 10;
	dateVal |= ((tens  << RTC_DATE_10YEAR_SHF) & RTC_DATE_10YEAR_MSK) |
		  (( single << RTC_DATE_YEAR_SHF) & RTC_DATE_YEAR_MSK);

#if defined(CONFIG_BUFFALO_PLATFORM)
	dateVal |= ((BuffaloFlag << RTC_DATE_BUFFALO_FLAG_SHF) & RTC_DATE_BUFFALO_FLAG_MSK);
#endif

	/* Update RTC Date Register */
	MV_REG_WRITE(RTC_DATE_REG, dateVal);


	return;
}

/*******************************************************************************
* mvRtcTimeGet - Read the time from the RTC.
*
* DESCRIPTION:
*       This function reads the current time and date from the RTC into the 
*       structure RTC_TIME (defined in mv.h).
*
* INPUT:
*       time - A pointer to a structure TIME (defined in mv.h).
*
* OUTPUT:
*       The structure RTC_TIME is updated with the time read from the RTC.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvRtcTimeGet(MV_RTC_TIME* mvTime)
{
	MV_U32 timeVal;
	MV_U32 dateVal;
	MV_U8 tens;
	MV_U8 single;

	/* read Time register */
	timeVal = MV_REG_READ(RTC_TIME_REG);
	
	/* seconds */
	tens = ((timeVal & RTC_TIME_10SECONDS_MSK) >> RTC_TIME_10SECONDS_SHF);
	single = ((timeVal & RTC_TIME_SECONDS_MSK) >> RTC_TIME_SECONDS_SHF);
	mvTime->seconds = 10*tens + single;

	/* minutes */
	tens = ((timeVal & RTC_TIME_10MINUTES_MSK) >> RTC_TIME_10MINUTES_SHF);
	single = ((timeVal & RTC_TIME_MINUTES_MSK) >> RTC_TIME_MINUTES_SHF);
	mvTime->minutes = 10*tens + single;

	/* hours */
	tens = ((timeVal & RTC_TIME_10HOUR_MSK) >> RTC_TIME_10HOUR_SHF);
	single = ((timeVal & RTC_TIME_HOUR_MSK) >> RTC_TIME_HOUR_SHF);
	mvTime->hours = 10*tens + single;

	/* day */
	mvTime->day = ((timeVal & RTC_TIME_DAY_MSK) >> RTC_TIME_DAY_SHF);
	mvTime->day--;

	/* read Date register */
	dateVal = MV_REG_READ(RTC_DATE_REG);

	/* day */
	tens = ((dateVal & RTC_DATE_10DAY_MSK) >> RTC_DATE_10DAY_SHF);
	single = ((dateVal & RTC_DATE_DAY_MSK) >> RTC_DATE_DAY_SHF);
	mvTime->date = 10*tens + single;

	/* month */
	tens = ((dateVal & RTC_DATE_10MONTH_MSK) >> RTC_DATE_10MONTH_SHF);
	single = ((dateVal & RTC_DATE_MONTH_MSK) >> RTC_DATE_MONTH_SHF);
	mvTime->month = 10*tens + single;

	/* year */
	tens = ((dateVal & RTC_DATE_10YEAR_MSK) >> RTC_DATE_10YEAR_SHF);
	single = ((dateVal  & RTC_DATE_YEAR_MSK) >> RTC_DATE_YEAR_SHF);
	mvTime->year = 10*tens + single;

	return;	
}

/*******************************************************************************
* mvRtcInit - Initialize the clock.
*
* DESCRIPTION:
*       This function initialize the RTC integrated unit.
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
MV_VOID mvRtcInit(MV_VOID)
{
	return;
}

#if defined(CONFIG_BUFFALO_PLATFORM)
MV_VOID mvRtcAlarmSet(MV_RTC_TIME* time)
{
	MV_U32 timeVal = 0;
	MV_U32 dateVal = 0;
	MV_U32 tens;
	MV_U32 single;
	
	/* seconds */
	tens = time->seconds / 10;
	single = time->seconds % 10;
	timeVal |= ((tens << RTC_TIME_10SECONDS_SHF) & RTC_TIME_10SECONDS_MSK) |
		  (( single << RTC_TIME_SECONDS_SHF) & RTC_TIME_SECONDS_MSK);

	/* minutes */
	tens = time->minutes / 10;
	single = time->minutes % 10;
	timeVal |= ((tens  << RTC_TIME_10MINUTES_SHF) & RTC_TIME_10MINUTES_MSK) |
		  (( single << RTC_TIME_MINUTES_SHF) & RTC_TIME_MINUTES_MSK);

	/* hours (24) */
	tens = time->hours / 10;
	single = time->hours % 10;
	timeVal |= ((tens  << RTC_TIME_10HOUR_SHF) & RTC_TIME_10HOUR_MSK) |
		  (( single  << RTC_TIME_HOUR_SHF) & RTC_TIME_HOUR_MSK);
  
	/* day */
	single = ++(time->day);
	timeVal |= ((single << RTC_TIME_DAY_SHF ) & RTC_TIME_DAY_MSK);

	/* Update RTC Time Register */
	MV_REG_WRITE(RTC_ALARM_TIME_REG, timeVal);

	/* date */
	tens = time->date / 10;
	single = time->date % 10;
	dateVal = ((tens  << RTC_DATE_10DAY_SHF) & RTC_DATE_10DAY_MSK) |
		  (( single << RTC_DATE_DAY_SHF) & RTC_DATE_DAY_MSK);

	/* month */
	tens = time->month / 10;
	single = time->month % 10;
	dateVal |= ((tens  << RTC_DATE_10MONTH_SHF) & RTC_DATE_10MONTH_MSK) |
		  (( single << RTC_DATE_MONTH_SHF) & RTC_DATE_MONTH_MSK);
    
	/* year */
	tens = time->year / 10;
	single = time->year % 10;
	dateVal |= ((tens  << RTC_DATE_10YEAR_SHF) & RTC_DATE_10YEAR_MSK) |
		  (( single << RTC_DATE_YEAR_SHF) & RTC_DATE_YEAR_MSK);

	/* Update RTC Date Register */
	MV_REG_WRITE(RTC_ALARM_DATE_REG, dateVal);


	return;
}


MV_VOID mvRtcAlarmGet(MV_RTC_TIME* time)
{
	MV_U32 timeVal;
	MV_U32 dateVal;
	MV_U8 tens;
	MV_U8 single;

	/* read Time register */
	timeVal = MV_REG_READ(RTC_ALARM_TIME_REG);
	
	/* seconds */
	tens = ((timeVal & RTC_TIME_10SECONDS_MSK) >> RTC_TIME_10SECONDS_SHF);
	single = ((timeVal & RTC_TIME_SECONDS_MSK) >> RTC_TIME_SECONDS_SHF);
	time->seconds = 10*tens + single;

	/* minutes */
	tens = ((timeVal & RTC_TIME_10MINUTES_MSK) >> RTC_TIME_10MINUTES_SHF);
	single = ((timeVal & RTC_TIME_MINUTES_MSK) >> RTC_TIME_MINUTES_SHF);
	time->minutes = 10*tens + single;

	/* hours */
	tens = ((timeVal & RTC_TIME_10HOUR_MSK) >> RTC_TIME_10HOUR_SHF);
	single = ((timeVal & RTC_TIME_HOUR_MSK) >> RTC_TIME_HOUR_SHF);
	time->hours = 10*tens + single;

	/* day */
	time->day = ((timeVal & RTC_TIME_DAY_MSK) >> RTC_TIME_DAY_SHF);
	time->day--;

	/* read Date register */
	dateVal = MV_REG_READ(RTC_ALARM_DATE_REG);

	/* day */
	tens = ((dateVal & RTC_DATE_10DAY_MSK) >> RTC_DATE_10DAY_SHF);
	single = ((dateVal & RTC_DATE_DAY_MSK) >> RTC_DATE_DAY_SHF);
	time->date = 10*tens + single;

	/* month */
	tens = ((dateVal & RTC_DATE_10MONTH_MSK) >> RTC_DATE_10MONTH_SHF);
	single = ((dateVal & RTC_DATE_MONTH_MSK) >> RTC_DATE_MONTH_SHF);
	time->month = 10*tens + single;

	/* year */
	tens = ((dateVal & RTC_DATE_10YEAR_MSK) >> RTC_DATE_10YEAR_SHF);
	single = ((dateVal  & RTC_DATE_YEAR_MSK) >> RTC_DATE_YEAR_SHF);
	time->year = 10*tens + single;

	return;	
}

MV_BOOL
bfGetBuffaloFlagAtIntegRtc(void)
{
	if(((MV_REG_READ(RTC_DATE_REG) & RTC_DATE_BUFFALO_FLAG_MSK) >> RTC_DATE_BUFFALO_FLAG_SHF) != 0)
	{
		return MV_TRUE;
	}
	return MV_FALSE;
}

MV_BOOL
bfSetBuffaloFlagAtIntegRtc(MV_U8 ope)
{
	MV_RTC_TIME mvTime, mvTimeModified;
	MV_U32 dateVal = 0;
	MV_U32 BuffaloFlag = 0;

	memset((void *)&mvTime, 0, sizeof(MV_RTC_TIME));
	memset((void *)&mvTimeModified, 0, sizeof(MV_RTC_TIME));
	mvRtcTimeGet(&mvTime);

	if(mvTime.hours == 23 && mvTime.minutes == 59 && mvTime.seconds >= 57)
	{
		msleep(5000);
		mvRtcTimeGet(&mvTime);
	}

	/* read Date register */
	dateVal = MV_REG_READ(RTC_DATE_REG);
	if(ope == SET_BUFFALO_FLAG)
		dateVal |= ((1 << RTC_DATE_BUFFALO_FLAG_SHF) & RTC_DATE_BUFFALO_FLAG_MSK);
	else if(ope == UNSET_BUFFALO_FLAG)
		dateVal &= ~((1 << RTC_DATE_BUFFALO_FLAG_SHF) & RTC_DATE_BUFFALO_FLAG_MSK);
	/* Update RTC Date Register */
	MV_REG_WRITE(RTC_DATE_REG, dateVal);

	// same date, and wanted flag value, then return with MV_TRUE;
        mvRtcTimeGet(&mvTimeModified);
	BuffaloFlag = bfGetBuffaloFlagAtIntegRtc();

	if((mvTimeModified.date == mvTime.date) && (mvTimeModified.month == mvTime.month) && (mvTimeModified.century == mvTime.century) && (mvTimeModified.year == mvTime.year))
	{
		if(ope == SET_BUFFALO_FLAG)
		{
			return BuffaloFlag;
		}
		else if(ope == UNSET_BUFFALO_FLAG)
		{
		if(BuffaloFlag == MV_TRUE)
			return MV_FALSE;
		else
			return MV_FALSE;
		}
		else
		{
			return MV_FALSE;
		}
	}
	else
	{
		return MV_FALSE;
	}

	return MV_FALSE;
}

MV_BOOL
bfBuffaloFlagAtIntegRtc(MV_U8 ope)
{
	switch(ope)
	{
	case SET_BUFFALO_FLAG:
	case UNSET_BUFFALO_FLAG:
		return bfSetBuffaloFlagAtIntegRtc(ope);
		break;
	case GET_BUFFALO_FLAG:
		return bfGetBuffaloFlagAtIntegRtc();
		break;
	default:
		return MV_FALSE;
	}
	return MV_FALSE;
}

#if defined(CONFIG_BUFFALO_USE_MICON)
bfUpsRecoverFlagAtIntegRtc(MV_U8 ope)
{
	return bfBuffaloFlagAtIntegRtc(ope);
}
#endif

MV_VOID bfSetMagicKey(MV_U32 key)
{
	printk("%s > Changed to 0x%02x from 0x%02x\n", __FUNCTION__, key, bfGetMagicKey());
	MV_REG_WRITE(RTC_ALARM_TIME_REG, key);

	switch(key)
	{
#if defined(CONFIG_BUFFALO_USE_MICON)
	case MagicKeyUpsShutdown:
	case MagicKeyUpsShutdownReachedHalt:
		bfBuffaloFlagAtIntegRtc(SET_BUFFALO_FLAG);
		break;
#else
	case MagicKeyRebootReachedHalt:
		bfBuffaloFlagAtIntegRtc(SET_BUFFALO_FLAG);
		break;
#endif
	default:
#if defined(CONFIG_BUFFALO_USE_MICON)
		bfBuffaloFlagAtIntegRtc(UNSET_BUFFALO_FLAG);
#endif
		break;
	}
}

MV_U32 bfGetMagicKey(MV_VOID)
{
	return MV_REG_READ(RTC_ALARM_TIME_REG);
}

#endif // defined(CONFIG_BUFFALO_PLATFORM)
