/*
 *  Driver routines for BUFFALO Platform
 *
 *  Copyright (C)  BUFFALO INC.
 *
 *  This software may be used and distributed according to the terms of
 *  the GNU General Public License (GPL), incorporated herein by reference.
 *  Drivers based on or derived from this code fall under the GPL and must
 *  retain the authorship, copyright and license notice.  This file is not
 *  a complete program and may only be used when the entire operating
 *  system is licensed under the GPL.
 *
 */


#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
#include <linux/ctype.h>
#endif
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include "boardEnv/mvBoardEnvLib.h"
#include "BuffaloGpio.h"
#include "buffalocore.h"
#include "buffalo/kernevnt.h"
#include "gpp/mvGppRegs.h"
#include "gpp/mvGpp.h"
#include "rtc/integ_rtc/mvRtc.h"
#include "BuffaloWol.h"
#include "buffalo_proc_entry.h"

/* Definitions */
#define BUFFALO_DRIVER_VER	"1.00"
#define DRIVER_NAME		"Buffalo Gpio Control Driver"
#define AUTHOR			"(C) BUFFALO INC."

#define USE_PROC_BUFFALO
#define BUFFALO_LED_CONTROL_INFO_BLOCK_ENABLE
	
#define MAX_ALARM_NUM	100
#define MAX_INFO_NUM	100
#define MAX_POWER_NUM	9
#define MAX_FUNC_NUM	9

#define BUFFALO_LED_WAIT_10		(HZ * 10 / 10)	// 1 sec
#define BUFFALO_LED_WAIT_1		(HZ * 2 / 10)	// 0.2 sec
#define BUFFALO_LED_WAIT_SHORT		(HZ * 3 / 10)	// 0.3 sec
#define BUFFALO_LED_WAIT_LONG		(HZ * 10 / 10)	// 1 sec


#define SW_POWER_NS	-1
#define SW_POWER_OFF	0
#define SW_POWER_ON	1
#define SW_POWER_AUTO	2

#define SW_POLLING_INTERVAL	1000	// 1sec

/* Definitions for DEBUG */
//#define DEBUG
#ifdef DEBUG
 #define FUNCTRACE(x)  x
#else
 #define FUNCTRACE(x) 

#endif

//#define LED_DEBUG
#if defined LED_DEBUG
  #define LED_TRACE(x)	x
#else
  #define LED_TRACE(x)
#endif

#if defined PST_DEBUG
  #define PST_TRACE(x)	x
#else
  #define PST_TRACE(x)
#endif

static struct timer_list BuffaloSwPollingTimer;
static unsigned int g_buffalo_sw_status = SW_POWER_NS;
static unsigned int g_buffalo_sw_polling_control = 0;

#ifdef CONFIG_ARCH_FEROCEON_MV78XX0

#include "BuffaloUart.h"

struct bfLedInfo alarmLed, infoLed, powerLed, funcLed;

#else

extern	MV_BOARD_INFO*	boardInfoTbl[];
#define BOARD_INFO(boardId)	boardInfoTbl[boardId - BUFFALO_BOARD_ID_BASE]

static void bfLedCodeDispStart(struct bfLedInfo *led);

#endif


// ----------------------------------------------------
static int
gpio_power_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len=0;

	if (BIT_PWR_SW < 0)
		return 0;

	if (bfGppInRegBitTest(BIT_PWR_SW))
		len = sprintf(page, "1\n");
	else
		len = sprintf(page, "0\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}


#ifndef CONFIG_ARCH_FEROCEON_MV78XX0
static int
bfIsLedActive(struct bfLedInfo *led)
{
	if (led->ledCodeInfo.code != 0 ||
	    led->ledCodeInfo.on_flag ||
	    led->ledCodeInfo.blink_flag)
		return 1;

	return 0;
}

static int
bfIsHighPriorityLedActive(struct bfLedInfo *led)
{
	int i;
	struct bfLedInfo *pli;
	MV_U32 boardId = mvBoardIdGet();
	
	for (i = 0; i < BOARD_INFO(boardId)->numLedInfo; i++) {
		pli = &BOARD_INFO(boardId)->pLedInfo[i];
		if (pli == led)
			continue;

		if (pli->group == led->group &&
		    pli->priority > led->priority &&
		    bfIsLedActive(pli)) {
			return 1;
		}
	}

	return 0;
}

static void
bfActivateLowPriorityLed(struct bfLedInfo *led)
{
	int i;
	struct bfLedInfo *pli;
	struct bfLedInfo *pfound_li = NULL;
	MV_U32 boardId = mvBoardIdGet();

	LED_TRACE(printk("%s > %s\n", __FUNCTION__, led->name));
	LED_TRACE(printk("%s > %s LED group = %d\n", __FUNCTION__, led->name, led->group));
	LED_TRACE(printk("%s > %s LED priority = %d\n", __FUNCTION__, led->name, led->priority));

	if (bfIsHighPriorityLedActive(led))
		return;

	for (i = 0; i < BOARD_INFO(boardId)->numLedInfo; i++) {
		pli = &BOARD_INFO(boardId)->pLedInfo[i];
		if (pli == led)
			continue;

		if (pli->group == led->group &&
		    pli->priority <= led->priority &&
		    bfIsLedActive(pli) &&
		    !bfIsLedActive(led)) {
			if (!pfound_li || pfound_li->priority <= pli->priority) {
				pfound_li = pli;
			}
		}
	}

	if (pfound_li && pfound_li->ledCodeInfo.blocked) {
		pfound_li->ledCodeInfo.blocked = 0;
		if (pfound_li->ledCodeInfo.code != 0) {
			bfLedCodeDispStart(pfound_li);
		}
		
		if (pfound_li->ledCodeInfo.on_flag) {
			bfGppOutRegBitAssert(pfound_li->gppPinNumber);
		}

		if (pfound_li->ledCodeInfo.blink_flag) {
			bfGppBlinkRegBitSet(pfound_li->gppPinNumber);
		}
	}

	return;
}

static void
bfDeactivateLowPriorityLed(struct bfLedInfo *led)
{
	int i;
	struct bfLedInfo *pli;
	MV_U32 boardId = mvBoardIdGet();

	LED_TRACE(printk("%s > Enter(LED %s)\n", __FUNCTION__, led->name));
	LED_TRACE(printk(" >>> group = %d, priority = %d\n", led->group, led->priority));

	for (i = 0; i < BOARD_INFO(boardId)->numLedInfo; i++) {
		pli = &BOARD_INFO(boardId)->pLedInfo[i];
		if (pli == led)
			continue;

		if (pli->ledCodeInfo.blocked) {
			LED_TRACE(printk(" >>> %s LED is blocked.\n", pli->name));
			continue;
		}

		LED_TRACE(printk(" >>> %s LED : group = %d, priority = %d, on_flag = %d, blink_flag = %d\n",
				 pli->name,
				 pli->group,
				 pli->priority,
				 pli->ledCodeInfo.on_flag,
				 pli->ledCodeInfo.bblink_flag));

		
		if (pli->group == led->group &&
		    pli->priority <= led->priority &&
		    bfIsLedActive(pli)) {
			LED_TRACE(printk(" >>> %s LED deactivate.\n", pli->name));
			pli->ledCodeInfo.blocked = 1;
			bfGppOutRegBitNagate(pli->gppPinNumber);
			bfGppBlinkRegBitClr(pli->gppPinNumber);
		}
	}
}
#endif


static void
BuffaloLed_StatChange(unsigned long data)
{
	struct bfLedInfo *led = (struct bfLedInfo *)data;

	LED_TRACE(printk("%s > Enter(LED %s)\n", __FUNCTION__, led->name));
	LED_TRACE(printk(" >>> tens_place_cnt = %d\n", led->ledCodeInfo.tens_place_cnt));
	LED_TRACE(printk(" >>> ones_place_cnt = %d\n", led->ledCodeInfo.ones_place_cnt));

	if (led->ledCodeInfo.tens_place_cnt) {
		// tens place disp
		if (bfIsGppOutRegBitAssert(led->gppPinNumber)) {
			bfGppOutRegBitNagate(led->gppPinNumber);
			led->ledCodeInfo.timer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
			led->ledCodeInfo.tens_place_cnt--;
		}
		else {
			bfGppOutRegBitAssert(led->gppPinNumber);
			led->ledCodeInfo.timer.expires = (jiffies + BUFFALO_LED_WAIT_10);
		}
	}
	else if (led->ledCodeInfo.ones_place_cnt) {
		// ones place disp
		if (bfIsGppOutRegBitAssert(led->gppPinNumber)) {
			bfGppOutRegBitNagate(led->gppPinNumber);
			led->ledCodeInfo.timer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
			led->ledCodeInfo.ones_place_cnt--;
		}
		else {
			bfGppOutRegBitAssert(led->gppPinNumber);
			led->ledCodeInfo.timer.expires = (jiffies + BUFFALO_LED_WAIT_1);
		}
	}
	else {
		led->ledCodeInfo.timer.expires = (jiffies + BUFFALO_LED_WAIT_LONG);
		led->ledCodeInfo.tens_place_cnt = led->ledCodeInfo.code / 10;
		led->ledCodeInfo.ones_place_cnt = led->ledCodeInfo.code % 10;
	}

	if (led->ledCodeInfo.blocked) {
		del_timer(&led->ledCodeInfo.timer);
		bfGppOutRegBitNagate(led->gppPinNumber);
	}
	else if (led->ledCodeInfo.code != 0) {
		add_timer(&led->ledCodeInfo.timer);
	}
	else {
		del_timer(&led->ledCodeInfo.timer);
		if (led->ledCodeInfo.on_flag) {
			bfGppOutRegBitAssert(led->gppPinNumber);
		}
		else {
			bfGppOutRegBitNagate(led->gppPinNumber);
		}
	}
}


static void
bfLedCodeDispStart(struct bfLedInfo *led)
{
	led->ledCodeInfo.tens_place_cnt = led->ledCodeInfo.code / 10;
	led->ledCodeInfo.ones_place_cnt = led->ledCodeInfo.code % 10;

	init_timer(&led->ledCodeInfo.timer);
	led->ledCodeInfo.timer.expires = (jiffies + BUFFALO_LED_WAIT_SHORT);
	led->ledCodeInfo.timer.function = BuffaloLed_StatChange;
	led->ledCodeInfo.timer.data = (unsigned long)led;
	add_timer(&led->ledCodeInfo.timer);
}


static void
bfLedCodeDispStop(struct bfLedInfo *led)
{
	led->ledCodeInfo.code = 0;
}


#ifndef CONFIG_ARCH_FEROCEON_MV78XX0
static int
BuffaloLedBlinkReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	struct bfLedInfo *led = data;

	if (bfGppBlinkRegBitTest(led->gppPinNumber)) {
		len += sprintf(page + len, "on\n");
	}
	else {
		if (led->ledCodeInfo.blink_flag)
			len += sprintf(page + len, "on(Blocked)\n");
		else
			len += sprintf(page + len, "off\n");
	}

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}


static int
BuffaloLedBlinkWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	struct bfLedInfo *led = data;
	LED_TRACE(printk("%s > Enter(LED %s)\n", __FUNCTION__, led->name));

	if(!strncmp(buffer, "on", 2)){
	  //		led->ledCodeInfo.on_flag = 0;
		led->ledCodeInfo.blink_flag = 1;
		bfLedCodeDispStop(led);

		if (!bfIsHighPriorityLedActive(led)) {
			bfGppBlinkRegBitSet(led->gppPinNumber);
			bfDeactivateLowPriorityLed(led);
		}
	}else if(!strncmp(buffer, "off", 3)){
	  //		led->ledCodeInfo.on_flag = 0;
		led->ledCodeInfo.blink_flag = 0;
		bfLedCodeDispStop(led);
		bfGppBlinkRegBitClr(led->gppPinNumber);
		bfActivateLowPriorityLed(led);
	}

	return count;
}
#endif

#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
static int
BuffaloAllLedReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (BIT_PWR_LED >= 0) {
		if (!bfGppOutRegBitTest(BIT_PWR_LED))
			len += sprintf(page + len, "power led :on\n");
		else
			len += sprintf(page + len, "power led :off\n");
	}

	if (BIT_INFO_LED >=0) {
		if (!bfGppOutRegBitTest(BIT_INFO_LED))
			len += sprintf(page + len, "info  led :on\n");
		else
			len += sprintf(page + len, "info  led :off\n");
	}

	if (BIT_ALARM_LED >= 0) {
		if (!bfGppOutRegBitTest(BIT_ALARM_LED))
			len += sprintf(page + len, "alarm led :on\n");
		else
			len += sprintf(page + len, "alarm led :off\n");
	}

	if (BIT_FUNC_LED >= 0) {
		if (!bfGppOutRegBitTest(BIT_FUNC_LED))
			len += sprintf(page + len, "func led :on\n");
		else
			len += sprintf(page + len, "func led :off\n");
	}

	if (len == 0)
		len += sprintf(page + len, "available led not found.\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}

static int
BuffaloAllLedWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
        if (strncmp(buffer, "on", 2) == 0 ) {
		BuffaloGpio_AllLedOn();
	} else if (strncmp(buffer, "off", 3) == 0) {
		BuffaloGpio_AllLedOff();
	}
	return count;
}
#endif


static int
BuffaloLedReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	struct bfLedInfo *led = data;
	int len = 0;

	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));

	if (led->ledCodeInfo.code == 0) {
		if (bfIsGppOutRegBitAssert(led->gppPinNumber)) {
			len += sprintf(page + len, "on\n");
		}
		else {
			if (led->ledCodeInfo.on_flag)
				len += sprintf(page + len, "on(Blocked)\n");
			else
				len += sprintf(page + len, "off\n");
		}
	}
	else {
		if (led->ledCodeInfo.blocked)
			len += sprintf(page + len, "%d(Blocked)\n", led->ledCodeInfo.code);
		else
			len += sprintf(page + len, "%d\n", led->ledCodeInfo.code);
	}


	if (len == 0)
		len += sprintf(page + len, "available led not found.\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
	return ((count < len - off) ? count : len - off);
}

static int
BuffaloLedWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	struct bfLedInfo *led = data;
	int code = simple_strtol(buffer, NULL, 10);

	LED_TRACE(printk("%s > Enter(LED %s)\n", __FUNCTION__, led->name));
	LED_TRACE(printk(">>> %s code = %d\n", led->name, code));

	if (code == 0) {
#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
		if (strncmp(buffer, "on", 2) == 0) {
			LED_TRACE(printk("%s > Calling BuffaloGpio_AlarmLedEnable\n", __FUNCTION__));
			bfGppOutRegBitClr(led->gppPinNumber);
		} else if (strncmp(buffer, "off", 3) == 0) {
			LED_TRACE(printk("%s > Calling BuffaloGpio_AlarmLedDisable\n", __FUNCTION__));
			bfGppBlinkRegBitClr(led->gppPinNumber);
			bfGppOutRegBitSet(led->gppPinNumber);
		} else if (strncmp(buffer, "inv", 3) == 0) {
			bfGppOutRegBitInv(led->gppPinNumber);
		}
#else
		if (!strncmp(buffer, "on", 2) && (led->ledCodeInfo.code == 0)) {
			led->ledCodeInfo.on_flag = 1;
			LED_TRACE(printk(">>> %s on.\n", led->name));
			if (!bfIsHighPriorityLedActive(led)) {
				bfGppOutRegBitAssert(led->gppPinNumber);
				bfDeactivateLowPriorityLed(led);
			}
		}
		else if (!strncmp(buffer, "off", 3) || !strncmp(buffer, "0", 1)) {
			if (led->ledCodeInfo.code != 0)
				bfLedCodeDispStop(led);

			led->ledCodeInfo.on_flag = 0;
			bfGppOutRegBitNagate(led->gppPinNumber);
			LED_TRACE(printk(">>> %s off.\n", led->name));
			bfActivateLowPriorityLed(led);
		}
#endif
	}
#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
	else
		if (led->ledCodeInfo.code == 0) {
			if ((code > 0) && (code <= MAX_ALARM_NUM)) {
				led->ledCodeInfo.code = code;
				bfLedCodeDispStart(led);
			} else if (code == 0) {
				bfLedCodeDispStop(led);
			} else {
				// Nothing to do...
			}
		}
#else
	else if ((0 < code) && (code <= MAX_ALARM_NUM)) {
		// display error/info code
		led->ledCodeInfo.on_flag = 0;
		bfGppOutRegBitNagate(led->gppPinNumber);

		if ((code == 52) && !strncmp(led->name, "info", 4)) {
			bfLedCodeDispStop(led);
			led->ledCodeInfo.on_flag = 1;
			if (!bfIsHighPriorityLedActive(led)) {
				bfGppOutRegBitAssert(led->gppPinNumber);
				bfDeactivateLowPriorityLed(led);
			}
		}
		else if (led->ledCodeInfo.code == 0) {
			led->ledCodeInfo.code = code;
			if (!bfIsHighPriorityLedActive(led)) {
				bfDeactivateLowPriorityLed(led);
				bfLedCodeDispStart(led);
			}
		}
		else {
			// overwrite error/info code
			led->ledCodeInfo.code = code;
		}
	}
#endif

	return count;
}

static int
BuffaloHddReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (bfGppOutRegBitTest(mvBoardGpioPinNumGet(BOARD_GPP_HDD_POWER, (int)data)))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}


static int
BuffaloHddWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0 ){
		bfGppOutRegBitAssert(mvBoardGpioPinNumGet(BOARD_GPP_HDD_POWER, (int)data));
	}else if(strncmp(buffer, "off", 3) == 0){
		bfGppOutRegBitNagate(mvBoardGpioPinNumGet(BOARD_GPP_HDD_POWER, (int)data));
	}

	return count;
}


static int
BuffaloUsbReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (bfGppOutRegBitTest(mvBoardGpioPinNumGet(BOARD_GPP_USB_VBUS_EN, (int)data)))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}


static int
BuffaloUsbWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0 ){
		bfGppOutRegBitAssert(mvBoardGpioPinNumGet(BOARD_GPP_USB_VBUS_EN, (int)data));
	}else if(strncmp(buffer, "off", 3) == 0){
		bfGppOutRegBitNagate(mvBoardGpioPinNumGet(BOARD_GPP_USB_VBUS_EN, (int)data));
	}

	return count;
}


static int
BuffaloPowerReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (bfGppInRegBitTest(BIT_PWR_SW))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}


static int
BuffaloAutoPowerReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (bfGppInRegBitTest(BIT_PWRAUTO_SW))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}


//#define SW_POLL_DEBUG
#ifdef SW_POLL_DEBUG
 #define SW_POLL_TRACE(x)	(x)
#else
 #define SW_POLL_TRACE(x)	/* */
#endif

static void
BuffaloSwPollingCheck(unsigned long arg)
{
	unsigned int present_sw_status = SW_POWER_NS;
	static char msg[32];

	SW_POLL_TRACE(printk("BIT_PWRAUTO_SW = %d\n", bfGppInRegBitTest(BIT_PWRAUTO_SW)));

	if (use_slide_power) {
		if (!(bfGppInRegBitTest(BIT_PWR_SW)) && !(bfGppInRegBitTest(BIT_PWRAUTO_SW)))
			present_sw_status = SW_POWER_OFF;
		else if ((bfGppInRegBitTest(BIT_PWR_SW)) && !(bfGppInRegBitTest(BIT_PWRAUTO_SW)))
			present_sw_status = SW_POWER_ON;
		else if (!(bfGppInRegBitTest(BIT_PWR_SW)) && (bfGppInRegBitTest(BIT_PWRAUTO_SW)))
			present_sw_status = SW_POWER_AUTO;
		else
			present_sw_status = SW_POWER_NS;
	}
	else {
		if (bfGppInRegBitTest(BIT_PWRAUTO_SW))
			present_sw_status = SW_POWER_AUTO;
		else
			present_sw_status = SW_POWER_ON;
	}

	SW_POLL_TRACE(printk("present_sw_status = 0x%08x\n", present_sw_status));
	SW_POLL_TRACE(printk("g_buffalo_sw_status = 0x%08x\n", g_buffalo_sw_status));
	
	if(g_buffalo_sw_status != present_sw_status)
	{
		g_buffalo_sw_status = present_sw_status;
		memset(msg, 0, sizeof(msg));
		switch(g_buffalo_sw_status)
		{
			case SW_POWER_OFF:
				SW_POLL_TRACE(printk("%s> SW_POWER_OFF\n", __FUNCTION__));
				buffalo_kernevnt_queuein("PSW_off");;
				break;
			case SW_POWER_ON:
				SW_POLL_TRACE(printk("%s> SW_POWER_ON\n", __FUNCTION__));
				buffalo_kernevnt_queuein("PSW_on");;
				break;
			case SW_POWER_AUTO:
				SW_POLL_TRACE(printk("%s> SW_POWER_AUTO\n", __FUNCTION__));
				buffalo_kernevnt_queuein("PSW_auto");;
				break;
			case SW_POWER_NS:
				SW_POLL_TRACE(printk("%s> SW_POWER_NS\n", __FUNCTION__));
				buffalo_kernevnt_queuein("PSW_unknown");;
				break;
			default:
				break;
		}
		SW_POLL_TRACE(printk("%s\n", msg));
	}

	BuffaloSwPollingTimer.expires = (jiffies + SW_POLLING_INTERVAL);
	add_timer(&BuffaloSwPollingTimer);
	return;
}

static int
BuffaloSwPollingStop(void)
{
	del_timer(&BuffaloSwPollingTimer);
	return 0;
}

static int
BuffaloSwPollingStart(void)
{
	init_timer(&BuffaloSwPollingTimer);
	BuffaloSwPollingTimer.expires = (jiffies + SW_POLLING_INTERVAL);
	BuffaloSwPollingTimer.function = &BuffaloSwPollingCheck;
	BuffaloSwPollingTimer.data = 0;
	add_timer(&BuffaloSwPollingTimer);
	return 0;
}


static int
BuffaloSwPollingWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	FUNCTRACE(printk("%s> Entered\n", __FUNCTION__));
	if(strncmp(buffer, "on", 2) == 0 )
	{
		if(g_buffalo_sw_polling_control == 0)
		{
			g_buffalo_sw_polling_control = 1;
			g_buffalo_sw_status = SW_POWER_NS;
			BuffaloSwPollingStart();
		}
	}
	else if(strncmp(buffer, "off", 3) == 0)
	{
		if(g_buffalo_sw_polling_control == 1)
		{
			g_buffalo_sw_polling_control = 0;
			BuffaloSwPollingStop();
			g_buffalo_sw_status = SW_POWER_NS;
		}
	}
	return count;
}

static int
BuffaloSwPollingReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	FUNCTRACE(printk("%s> Entered\n", __FUNCTION__));

	if(g_buffalo_sw_polling_control == 1)
	{
		len = sprintf(page, "on\n");
	}
	else
	{
		len = sprintf(page, "off\n");
	}

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}



static int
BuffaloFuncReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (bfGppInRegBitTest(BIT_FUNC_SW))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}



static int
BuffaloFanStatusReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (BIT_FAN_LOCK < 0)
		return 0;

	if (bfGppInRegBitTest(BIT_FAN_LOCK))
		len = sprintf(page, "Stop\n");
	else
		len = sprintf(page, "Fine\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}



static int
BuffaloFanStatusWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0){
		//Nothing to do ...
	}else if(strncmp(buffer, "off", 3) == 0){
		//Nothing to do ...
	}
	return count;
}



static int
BuffaloFanControlWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "stop", strlen("stop")) == 0)
	{
		bfGppOutRegBitNagate(BIT_FAN_LOW);
		bfGppOutRegBitNagate(BIT_FAN_HIGH);
	}
	else if(strncmp(buffer, "slow", strlen("slow")) == 0)
	{
		bfGppOutRegBitAssert(BIT_FAN_LOW);
		bfGppOutRegBitNagate(BIT_FAN_HIGH);
	}
	else if(strncmp(buffer, "fast", strlen("fast")) == 0)
	{
		bfGppOutRegBitNagate(BIT_FAN_LOW);
		bfGppOutRegBitAssert(BIT_FAN_HIGH);
	}
	else if(strncmp(buffer, "full", strlen("full")) == 0)
	{
		bfGppOutRegBitAssert(BIT_FAN_LOW);
		bfGppOutRegBitAssert(BIT_FAN_HIGH);
	}

	return count;
}

static int
BuffaloFanControlReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (!bfGppInRegBitTest(BIT_FAN_LOW) && !bfGppInRegBitTest(BIT_FAN_HIGH)) {
		len += sprintf(page, "stop\n");
	}
	else if (bfGppInRegBitTest(BIT_FAN_LOW) && !bfGppInRegBitTest(BIT_FAN_HIGH)) {
		len += sprintf(page, "slow\n");
	}
	else if (!bfGppInRegBitTest(BIT_FAN_LOW) && bfGppInRegBitTest(BIT_FAN_HIGH)) {
		len += sprintf(page, "fast\n");
	}
	else {
		len += sprintf(page, "full\n");
	}

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}

#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
static int
BuffaloKatoi(const char *buffer, unsigned int count){
#define MAX_LENGTH      16
	int i;
	int TmpNum=0;

	LED_TRACE(printk("%s > Entered\n", __FUNCTION__));

	if (count > MAX_LENGTH) {
		LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
		return -1;
	}

	LED_TRACE(printk("%s > buffer=0x%08x\n", __FUNCTION__, buffer));
	LED_TRACE(printk("%s > *buffer=0x%08x\n", __FUNCTION__, *buffer));
	for (i = 0; i < count; i++, buffer++) {
		LED_TRACE(printk("%s > *buffer=0x%08x, TmpNum=%d\n", __FUNCTION__, *buffer, TmpNum));
												if (*buffer >= '0' && *buffer <= '9')
			TmpNum = (TmpNum * 10) + (*buffer - '0');
	}

	LED_TRACE(printk("%s > Leaving\n", __FUNCTION__));
	return TmpNum;
}

static int
BuffaloFanSenseChannelReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	int i = 0;
	int present_channel = -1;

	for (i = 0; mvBoardGpioPinNumGet(BOARD_GPP_FAN_SELECT, i) != -1; i++) {
		if (bfGppOutRegBitTest(mvBoardGpioPinNumGet(BOARD_GPP_FAN_SELECT, i))) {
			if(present_channel == -1)
				present_channel = i;
			else
				// in the case, *many* fan sence pins are settedto 1
				present_channel = -1;
		}
	}

	len += sprintf(page, "%d\n", present_channel);

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}

static int
BuffaloFanSenseChannelWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	int i = 0;
	int active_channel = -1;
	int requested_channel = -1;

	if(isdigit(buffer[0]) == 0)
		return count;

	requested_channel = BuffaloKatoi(buffer, count);

	for (i = 0; mvBoardGpioPinNumGet(BOARD_GPP_FAN_SELECT, i) != -1; i++) {
		if (i == requested_channel) {
			active_channel = i;
			break;
		}
	}

        if(active_channel == -1)
		return count;

	for (i = 0; mvBoardGpioPinNumGet(BOARD_GPP_FAN_SELECT, i) != -1; i++)
		bfGppOutRegBitClr(mvBoardGpioPinNumGet(BOARD_GPP_FAN_SELECT, i));

	bfGppOutRegBitSet(mvBoardGpioPinNumGet(BOARD_GPP_FAN_SELECT, active_channel));

	return count;
}

static int
BuffaloModemControlForceTxReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	if (bfGppOutRegBitTest(BIT_UPS_TXD))
		len += sprintf(page, "1\n");
	else
		len += sprintf(page, "0\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}

static int
BuffaloModemControlForceTxWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if (strncmp(buffer, "1", strlen("1")) == 0) {
		bfGppOutRegBitSet(BIT_UPS_TXD);
	} else if(strncmp(buffer, "0", strlen("0")) == 0) {
		bfGppOutRegBitClr(BIT_UPS_TXD);
	}
	return count;
}

static int
BuffaloModemControlReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	unsigned int regval = bfRegGet(GPP_MPP_CTL(3), 0x0000FFFF);

	if (regval == 0x00004444)
		len += sprintf(page, "mpp\n");
	else if (regval == 0x00000000)
		len += sprintf(page, "gpio\n");
	else
		len += sprintf(page, "unknown\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}

static int
BuffaloModemControlWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if (strncmp(buffer, "gpio", strlen("gpio")) == 0) {
		// change mpp pin setting to gpio mode;
		bfRegSet(GPP_MPP_CTL(3), 0x0000FFFF, 0x00000000);
		BuffaloMctrlMode = BUFFALO_MODEM_CTRL_MODE_GPIO;
	} else if (strncmp(buffer, "mpp", strlen("mpp")) == 0) {
		// change mpp pin setting to mpp mode;
		bfRegSet(GPP_MPP_CTL(3), 0x0000FFFF, 0x00004444);
		BuffaloMctrlMode = BUFFALO_MODEM_CTRL_MODE_MPP;
	}
	return count;
}
#endif

static int
BuffaloCpuStatusReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	unsigned int CpuStatus;

#ifdef CONFIG_ARCH_FEROCEON_KW
	CpuStatus = bfGetMagicKey();
#else
	CpuStatus = BuffaloGpio_ChangePowerStatus(0);  
#endif

	switch(CpuStatus){
	case MagicKeyReboot:
		len = sprintf(page, "reboot\n");
		break;
	case MagicKeyRebootUbootPassed:
		len = sprintf(page, "reboot_uboot_passed\n");
		break;
	case MagicKeyNormalState:
		len = sprintf(page, "normal_state\n");
		break;
	case MagicKeyHwPoff:
		len = sprintf(page, "hwpoff\n");
		break;
	case MagicKeySwPoff:
		len = sprintf(page, "swpoff\n");
		break;
	case MagicKeySWPoffUbootPassed:
		len = sprintf(page, "swpoff_uboot_passed\n");
		break;
	case MagicKeyFWUpdating:
		len = sprintf(page, "fwup\n");
		break;
	case MagicKeyUpsShutdown:
		len = sprintf(page, "ups_shutdown\n");
		break;
#ifdef CONFIG_ARCH_FEROCEON_KW
	case MagicKeyWOLReadyState:
		len = sprintf(page, "WOLReady\n");
		break;
#else
	case MagicKeyWOLReadyState:
		len = sprintf(page, "wol_ready\n");
		break;
#endif
	case MagicKeyWOLReadyUbootPasswd:
		len = sprintf(page, "wol_ready_uboot_passed\n");
		break;
	default:
		len = sprintf(page, "Unknown(CpuStatus=%d)\n", CpuStatus);
		break;
	}

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}

static int BuffaloCpuStatusWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	char status[256];
	if (count > 256)
		return count;

	memcpy(status, buffer, count);

	if (status[count - 1] == '\n')
		status[count - 1] = '\0';
	else
		status[count] = '\0';

	if(strcmp(status, "reboot") == 0){
		PST_TRACE(printk("%s > setting reboot\n", __FUNCTION__));
#ifdef CONFIG_ARCH_FEROCEON_KW
		bfSetMagicKey(MagicKeyReboot);
#else
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_REBOOTING);
#endif
	}else if(strcmp(status, "reboot_uboot_passed") == 0){
		PST_TRACE(printk("%s > setting Reboot_uboot_passed\n", __FUNCTION__));
#ifdef CONFIG_ARCH_FEROCEON_KW
		bfSetMagicKey(MagicKeyRebootUbootPassed);
#else
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_REBOOTING_UBOOT_PASSED);
#endif
	}else if(strcmp(status, "normal_state") == 0){
		PST_TRACE(printk("%s > setting normal_state\n", __FUNCTION__));
#ifdef CONFIG_ARCH_FEROCEON_KW
		bfSetMagicKey(MagicKeyNormalState);
#else
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_NORMAL_STATE);
#endif
	}else if(strcmp(status, "hwpoff") == 0){
		PST_TRACE(printk("%s > setting hwpoff\n", __FUNCTION__));
#ifdef CONFIG_ARCH_FEROCEON_KW
		bfSetMagicKey(MagicKeyHwPoff);
#else
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_HW_POWER_OFF);
#endif
	}else if(strcmp(status, "swpoff") == 0){
		PST_TRACE(printk("%s > setting swpoff\n", __FUNCTION__));
#ifdef CONFIG_ARCH_FEROCEON_KW
		if (bfGetMagicKey() != MagicKeyWOLReadyState) {
			bfSetMagicKey(MagicKeySwPoff);
		}
#else
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_SW_POWER_OFF);
#endif
	}else if(strcmp(status, "swpoff_uboot_passed") == 0){
		PST_TRACE(printk("%s > setting swpoff_uboot_passed\n", __FUNCTION__));
#ifdef CONFIG_ARCH_FEROCEON_KW
		bfSetMagicKey(MagicKeySWPoffUbootPassed);
#else
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_SW_POFF_UBOOT_PASSED);
#endif
	}else if(strcmp(status, "fwup") == 0){
		PST_TRACE(printk("%s > setting fwup\n", __FUNCTION__));
#ifdef CONFIG_ARCH_FEROCEON_KW
		bfSetMagicKey(MagicKeyFWUpdating);
#else
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_FWUPDATING);
#endif
#ifndef CONFIG_ARCH_FEROCEON_KW
	}else if(strncmp(buffer, "wol_ready_uboot_passed", 22) == 0){
		PST_TRACE(printk("%s > setting wol_ready_uboot_passed\n", __FUNCTION__));
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_WOL_READY_UBOOT_PASSED);
#endif
	}else if(strcmp(status, "ups_shutdown") == 0){
		PST_TRACE(printk("%s > setting ups_shutdown\n", __FUNCTION__));
#ifdef CONFIG_ARCH_FEROCEON_KW
		bfSetMagicKey(MagicKeyUpsShutdown);
	}else if(strcmp(status, "WOLReady") == 0){
		bfSetMagicKey(MagicKeyWOLReadyState);
		bfControlWolInterrupt(0, FLAG_BUFFALO_WOL_INTERRUPT_DISABLE);
		bfResetEthPhy(0);
		bfSetWolFrameWaitMode(0, 0);
#else
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_UPS_SHUTDOWN);
	}else if(strncmp(buffer, "wol_ready", 9) == 0){
		PST_TRACE(printk("%s > setting wol_ready\n", __FUNCTION__));
		BuffaloGpio_ChangePowerStatus(POWER_STATUS_WOL_READY_STATE);
#endif
	}
	else{
		PST_TRACE(printk("%s > no meaning...(%s)\n", __FUNCTION__, status));
	}

	return count;
}

static int
bfRegReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;

	len += sprintf(page, "0x%08x\n", MV_REG_READ((MV_U32)data));

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}

static int
bfRegWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	MV_REG_WRITE((MV_U32)data, simple_strtoul(buffer, NULL, 16));
	return count;
}


#if 0
static int
BuffaloPMHddPowerReadProc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	uint32_t pm_hdd_power;
	if (BuffaloReadPmGpio(&pm_hdd_power) != 0)
		return 0;

	pm_hdd_power = (pm_hdd_power & 0xff00) >> 8;

	if (pm_hdd_power & BIT((int)data))
		return sprintf(page, "on\n");
	else
		return sprintf(page, "off\n");
}


static int
BuffaloPMHddPowerWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	uint32_t pm_hdd_power;
	
	if (BuffaloReadPmGpio(&pm_hdd_power) != 0)
		return 0;

	if (strncmp(buffer, "on", 2) == 0 ){
		pm_hdd_power |= BIT((int)data + 8);
	}
	else if(strncmp(buffer, "off", 3) == 0){
		pm_hdd_power &= ~BIT((int)data + 8);
	}

	if (BuffaloWritePmGpio(&pm_hdd_power) != 0)
		return 0;

	return count;
}


static int
BuffaloPMHddDiagLedReadProc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	uint32_t pm_diag_led;
	
	if (BuffaloReadPmGpio(&pm_diag_led) != 0)
		return 0;

	pm_diag_led = (pm_diag_led & 0xff00) >> 12;

	if (pm_diag_led & BIT((int)data))
		return sprintf(page, "off\n");
	else
		return sprintf(page, "on\n");
}


static int
BuffaloPMHddDiagLedWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	uint32_t pm_diag_led;

	if (BuffaloReadPmGpio(&pm_diag_led) != 0)
		return 0;

	if (strncmp(buffer, "on", 2) == 0 ){
		pm_diag_led &= ~BIT((int)data + 12);
	}
	else if(strncmp(buffer, "off", 3) == 0){
		pm_diag_led |= BIT((int)data + 12);
	}

	if (BuffaloWritePmGpio(&pm_diag_led) != 0)
		return 0;

	return count;
}
#endif


struct bf_proc_entry_tab proc_ent_tab[] = {
  {"buffalo/gpio/fan",              1,NULL,                    NULL,                     NULL,     -1},
#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
  {"buffalo/gpio/fan/sense_channel",0,BuffaloFanSenseChannelReadProc, BuffaloFanSenseChannelWriteProc, NULL, -1},
#endif
  {"buffalo/gpio/power_control",    1,NULL,                    NULL,                     NULL,     -1},
  {"buffalo/cpu_status",            0,BuffaloCpuStatusReadProc,BuffaloCpuStatusWriteProc,NULL,     -1},
  {"buffalo/power_sw",              0,gpio_power_read_proc,    NULL,                     NULL,     -1},
#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
  {"buffalo/gpio/led",              1,NULL,                    NULL,                     NULL,     -1},
  {"buffalo/gpio/led/all",          0,BuffaloAllLedReadProc,   BuffaloAllLedWriteProc,   NULL,     -1},
  {"buffalo/gpio/led/alarm",        0,BuffaloLedReadProc,      BuffaloLedWriteProc,      &alarmLed,BOARD_GPP_ALARM_LED},
  {"buffalo/gpio/led/alarm_blink",  0,BuffaloLedReadProc,      BuffaloLedWriteProc,      &alarmLed,BOARD_GPP_ALARM_LED},
  {"buffalo/gpio/led/info",         0,BuffaloLedReadProc,      BuffaloLedWriteProc,      &infoLed, BOARD_GPP_INFO_LED},
  {"buffalo/gpio/led/info_blink",   0,BuffaloLedReadProc,      BuffaloLedWriteProc,      &infoLed, BOARD_GPP_INFO_LED},
  {"buffalo/gpio/led/power",        0,BuffaloLedReadProc,      BuffaloLedWriteProc,      &powerLed,BOARD_GPP_PWR_LED},
  {"buffalo/gpio/led/power_blink",  0,BuffaloLedReadProc,      BuffaloLedWriteProc,      &powerLed,BOARD_GPP_PWR_LED},
  {"buffalo/gpio/led/func",         0,BuffaloLedReadProc,      BuffaloLedWriteProc,      &funcLed, BOARD_GPP_FUNC_LED},
  {"buffalo/gpio/led/func_blink",   0,BuffaloLedReadProc,      BuffaloLedWriteProc,      &funcLed, BOARD_GPP_FUNC_LED},
#endif
  {"buffalo/gpio/switch",           1,NULL,                    NULL,                     NULL,     -1},
  {"buffalo/gpio/switch/auto_power",0,BuffaloAutoPowerReadProc,NULL,                     NULL,     BOARD_GPP_PWRAUTO_SW},
  {"buffalo/gpio/switch/sw_control",0,BuffaloSwPollingReadProc,BuffaloSwPollingWriteProc,NULL,     BOARD_GPP_PWRAUTO_SW},
  {"buffalo/gpio/switch/func",      0,BuffaloFuncReadProc     ,NULL,                     NULL,     BOARD_GPP_FUNC_SW},
  {"buffalo/gpio/fan/lock",         0,BuffaloFanStatusReadProc,BuffaloFanStatusWriteProc,NULL,     BOARD_GPP_FAN_LOCK},
  {"buffalo/regs",                  1,NULL,                    NULL,                     NULL,     -1},
  {"buffalo/regs/gpp",              1,NULL,                    NULL,                     NULL,     -1},
  {"buffalo/regs/gpp/data_out",     0,bfRegReadProc,           bfRegWriteProc,          (void*)GPP_DATA_OUT_REG(0),-1},
  {"buffalo/regs/gpp/high_data_out",0,bfRegReadProc,           bfRegWriteProc,          (void*)GPP_DATA_OUT_REG(1),-1},
  {"buffalo/regs/gpp/data_out_enable",0,bfRegReadProc,         bfRegWriteProc,          (void*)GPP_DATA_OUT_EN_REG(0),-1},
  {"buffalo/regs/gpp/high_data_out_enable",0,bfRegReadProc,    bfRegWriteProc,          (void*)GPP_DATA_OUT_EN_REG(1),-1},
  {"buffalo/regs/gpp/blink_enable", 0,bfRegReadProc,           bfRegWriteProc,          (void*)GPP_BLINK_EN_REG(0),-1},
  {"buffalo/regs/gpp/high_blink_enable",0,bfRegReadProc,       bfRegWriteProc,          (void*)GPP_BLINK_EN_REG(1),-1},
  {"buffalo/regs/gpp/data_in_polarity",0,bfRegReadProc,        bfRegWriteProc,          (void*)GPP_DATA_IN_POL_REG(0),-1},
  {"buffalo/regs/gpp/high_data_in_polarity",0,bfRegReadProc,   bfRegWriteProc,          (void*)GPP_DATA_IN_POL_REG(1),-1},
  {"buffalo/regs/gpp/data_in",      0,bfRegReadProc,           bfRegWriteProc,          (void*)GPP_DATA_IN_REG(0),-1},
  {"buffalo/regs/gpp/high_data_in", 0,bfRegReadProc,           bfRegWriteProc,          (void*)GPP_DATA_IN_REG(1),-1},
  {"buffalo/regs/gpp/int_cause",    0,bfRegReadProc,           bfRegWriteProc,          (void*)GPP_INT_CAUSE_REG(0),-1},
  {"buffalo/regs/gpp/cpu0_edge_int_mask", 0,bfRegReadProc,     bfRegWriteProc,          (void*)GPP_INT_MASK_REG(0),-1},
  {"buffalo/regs/gpp/cpu0_level_int_mask",0,bfRegReadProc,     bfRegWriteProc,          (void*)GPP_INT_LVL_REG(0),-1},
#if 0
  {"buffalo/regs/int",              1,NULL,                    NULL,                     NULL,     -1},
  {"buffalo/regs/int/main_int_low", 0,bfRegReadProc,           bfRegWriteProc,          (void*)CPU_INT_LOW_REG(0), -1},
  {"buffalo/regs/int/main_int_high",0,bfRegReadProc,           bfRegWriteProc,          (void*)CPU_INT_HIGH_REG(0), -1},
  {"buffalo/regs/int/irq_int_mask_low",0,bfRegReadProc,        bfRegWriteProc,          (void*)CPU_INT_MASK_LOW_REG(0), -1},
  {"buffalo/regs/int/irq_int_mask_high",0,bfRegReadProc,       bfRegWriteProc,          (void*)CPU_INT_MASK_HIGH_REG(0), -1},
#endif
#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
  {"buffalo/gpio/mctrl",            1,NULL,                    NULL,
	   NULL,     -1},
  {"buffalo/gpio/mctrl/mctrl_mode", 0,BuffaloModemControlReadProc, BuffaloModemControlWriteProc, NULL, -1},
  {"buffalo/gpio/mctrl/force_tx", 0,BuffaloModemControlForceTxReadProc, BuffaloModemControlForceTxWriteProc, NULL, -1},
#endif
  {NULL,                            0,NULL,                    NULL,                     NULL,     -1},
};
  
static void BuffaloProcEntryInit(void)
{
	int i;
	struct proc_dir_entry *ent;

	for (i = 0; proc_ent_tab[i].name != NULL; i++) {
		if (proc_ent_tab[i].isDirectory) {
			proc_mkdir(proc_ent_tab[i].name, 0);
		}
		else {
			if ((proc_ent_tab[i].class != -1) &&
			    (mvBoardGpioPinNumGet(proc_ent_tab[i].class, 0) == MV_ERROR)) {
				continue;
			}

			ent = create_proc_entry(proc_ent_tab[i].name, 0, 0);
			if (!ent) continue;

			if (proc_ent_tab[i].read_proc)
				ent->read_proc = proc_ent_tab[i].read_proc;

			if (proc_ent_tab[i].write_proc)
				ent->write_proc = proc_ent_tab[i].write_proc;

			ent->data = proc_ent_tab[i].data;
		}
	}
}

static void BuffaloHddPowerProcInit(void)
{
	int i;
	char buf[64];
	struct proc_dir_entry *ent;

	for (i=0; mvBoardGpioPinNumGet(BOARD_GPP_HDD_POWER, i) != -1; i++) {
		sprintf(buf, "buffalo/gpio/power_control/hdd%d", i);
		ent = create_proc_entry(buf, 0, 0);
		if (ent != NULL) {
			ent->read_proc = BuffaloHddReadProc;
			ent->write_proc= BuffaloHddWriteProc;
			ent->data = (void *)i;
		}
	}
}

static void BuffaloUsbPowerProcInit(void)
{
	int i;
	char buf[64];
	struct proc_dir_entry *ent;

	for (i=0; mvBoardGpioPinNumGet(BOARD_GPP_USB_VBUS_EN, i) != -1; i++) {
		sprintf(buf, "buffalo/gpio/power_control/usb%d", i);
		ent = create_proc_entry(buf, 0, NULL);
		if (ent != NULL) {
			ent->read_proc = BuffaloUsbReadProc;
			ent->write_proc = BuffaloUsbWriteProc;
			ent->data = (void *)i;
		}
	}
}

#ifdef CONFIG_ARCH_FEROCEON_KW
static void BuffaloLedInit(void)
{
	MV_U32 boardId = mvBoardIdGet();
	int i;
	struct proc_dir_entry *ent, *parent_ent;
	char buf[32];

	LED_TRACE(printk("%s > Entered.\n",__FUNCTION__));

	if (!((boardId >= BOARD_ID_BASE)&&(boardId < MV_MAX_BOARD_ID)))	{
		printk("%s:Board unknown.\n", __FUNCTION__);
		return;
	}

	parent_ent = proc_mkdir("buffalo/gpio/led", 0);
	if (!parent_ent)
		return;

	for (i = 0; i < BOARD_INFO(boardId)->numLedInfo; i++) {
		ent = create_proc_entry(BOARD_INFO(boardId)->pLedInfo[i].name, S_IRUSR|S_IWUSR, parent_ent);
		if (ent) {
			ent->read_proc = BuffaloLedReadProc;
			ent->write_proc = BuffaloLedWriteProc;
			ent->data = (void *)(&BOARD_INFO(boardId)->pLedInfo[i]);
		}

		sprintf(buf, "%s_blink", BOARD_INFO(boardId)->pLedInfo[i].name);
		ent = create_proc_entry(buf, S_IRUSR|S_IWUSR, parent_ent);
		if (ent) {
			ent->read_proc = BuffaloLedBlinkReadProc;
			ent->write_proc = BuffaloLedBlinkWriteProc;
			ent->data = (void *)(&BOARD_INFO(boardId)->pLedInfo[i]);
		}

		if (bfIsGppOutRegBitAssert(BOARD_INFO(boardId)->pLedInfo[i].gppPinNumber))
			BOARD_INFO(boardId)->pLedInfo[i].ledCodeInfo.on_flag = 1;
		else
			BOARD_INFO(boardId)->pLedInfo[i].ledCodeInfo.on_flag = 0;

		if (bfGppBlinkRegBitTest(BOARD_INFO(boardId)->pLedInfo[i].gppPinNumber))
			BOARD_INFO(boardId)->pLedInfo[i].ledCodeInfo.blink_flag = 1;
		else
			BOARD_INFO(boardId)->pLedInfo[i].ledCodeInfo.blink_flag = 0;

		LED_TRACE(printk("%s > %s LED on_flag = %d\n",
				 __FUNCTION__,
				 BOARD_INFO(boardId)->pLedInfo[i].name,
				 BOARD_INFO(boardId)->pLedInfo[i].ledCodeInfo.on_flag));
		LED_TRACE(printk("%s > %s LED blink_flag = %d\n",
				 __FUNCTION__,
				 BOARD_INFO(boardId)->pLedInfo[i].name,
				 BOARD_INFO(boardId)->pLedInfo[i].ledCodeInfo.blink_flag));
	}
}
#endif

//----------------------------------------------------------------------
int __init buffaloGpioDriver_init (void)
{
	struct proc_dir_entry *gpio;
	struct proc_dir_entry *ent;

	FUNCTRACE(printk("%s > Entered.\n",__FUNCTION__));
	if(mvBoardGpioPinNumGet(BOARD_GPP_MC_IRQ, 0) != -1)
	{
		use_slide_power = 0;
	}

	//	BuffaloGpio_FanControlWrite(BUFFALO_GPIO_FAN_SLOW);
	BuffaloFanControlWriteProc(NULL, "slow", 0, NULL);

	gpio = proc_mkdir("gpio", get_proc_buffalo());

	BuffaloProcEntryInit();
	BuffaloHddPowerProcInit();
	BuffaloUsbPowerProcInit();
#ifdef CONFIG_ARCH_FEROCEON_KW
	BuffaloLedInit();
#endif

	if (use_slide_power) {
		ent = create_proc_entry("buffalo/gpio/switch/power", 0, 0);
		ent->read_proc = BuffaloPowerReadProc;
	}

	if (BIT_FAN_LOW >= 0 && BIT_FAN_HIGH >= 0) {
		ent = create_proc_entry("buffalo/gpio/fan/control", 0, 0);
		ent->read_proc = BuffaloFanControlReadProc;
		ent->write_proc= BuffaloFanControlWriteProc;
	}

#if 0
	if (Buffalo_has_PM()) {
		for (i=0; i<4; i++) {
			sprintf(buf, "buffalo/gpio/power_control/hdd%d", i);
			pde = create_proc_entry(buf, 0, 0);
			if (pde != NULL) {
				pde->read_proc = BuffaloPMHddPowerReadProc;
				pde->write_proc= BuffaloPMHddPowerWriteProc;
				pde->data = (void *)i;
			}

			sprintf(buf, "buffalo/gpio/led/pm_diag_led%d", i);
			pde = create_proc_entry(buf, 0, 0);
			if (pde != NULL) {
				pde->read_proc = BuffaloPMHddDiagLedReadProc;
				pde->write_proc= BuffaloPMHddDiagLedWriteProc;
				pde->data = (void *)i;
			}
		}
	}
#endif

	printk("%s %s Ver.%s installed.\n", DRIVER_NAME, AUTHOR, BUFFALO_DRIVER_VER);
	return 0;
}

//----------------------------------------------------------------------
void buffaloGpioDriver_exit(void)
{
	FUNCTRACE(printk(">%s\n",__FUNCTION__));
}

module_init(buffaloGpioDriver_init);
module_exit(buffaloGpioDriver_exit);

/* Module parameters */
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DRIVER_NAME);
MODULE_LICENSE("GPL");
#if 0 /* LINUX_VERSION_CODE < KERNEL_VERSION(3,3,4) */
MODULE_VERSION(BUFFALO_DRIVER_VER);
#endif

