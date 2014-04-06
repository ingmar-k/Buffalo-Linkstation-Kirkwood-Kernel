
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
#include <linux/config.h>
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
#include <linux/sched.h>
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/irq.h>

#include "boardEnv/mvBoardEnvLib.h"
#include "buffalocore.h"
#include "buffalo/kernevnt.h"
#include "BuffaloGpio.h"
#include "buffalo_proc_entry.h"
#include "gpp/mvGppRegs.h"

#define AUTHOR	"(C) BUFFALO INC."
#define DRIVER_NAME	"Buffalo CPU Inerupts Driver"
#define BUFFALO_DRIVER_VER	"0.01 alpha1"

MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DRIVER_NAME);
MODULE_LICENSE("GPL");
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,4)
MODULE_VERSION(BUFFALO_DRIVER_VER);
#endif

#define CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER

//#define CONFIG_BUFFALO_DEBUG_GPIO_INTERRUPT_DRIVER
#ifdef CONFIG_BUFFALO_DEBUG_GPIO_INTERRUPT_DRIVER
	#define TRACE(x)	x
#else
	#define TRACE(x)
#endif

#define POLLING_COUNTS_PER_1SEC	10

#define SW_PUSHED		(1 << 0)	// 0x01
#define SW_LONG_PUSHED		(1 << 1)	// 0x02


static int g_irq=0;
static unsigned int power_pushed_status;

#define PSW_IRQ	(IRQ_GPP_START + BIT_PWR_SW)
#define PSW_POL_MSEC	(HZ / POLLING_COUNTS_PER_1SEC) 	// 50 mili second
#define PSW_WAIT_SECOND	(1 * HZ)			// 3 seccond
#define PSW_LONG_WAIT_COUNT	(PSW_WAIT_SECOND / PSW_POL_MSEC)	// PSW_WAIT_SECOND / PSW_POL_MSEC

#define PSW_WAIT_COUNT	(0 * PSW_POL_MSEC) // 0.3(6 * 0.05) second wait

static struct timer_list PSWPollingTimer;
static void PollingTimerGoOn(void);
static void PollingTimerStop(void);


// for init sw
static int g_irq_init=0;
static unsigned int init_pushed_status;

#define INIT_IRQ (IRQ_GPP_START + BIT_INIT_SW)

#define INIT_POL_MSEC	(HZ / POLLING_COUNTS_PER_1SEC)
#define INIT_WAIT_SECOND	(5 * HZ)
#define INIT_WAIT_COUNT	(INIT_WAIT_SECOND / INIT_POL_MSEC)

static struct timer_list INITPollingTimer;
static void PollingTimerGoOnInit(void);
static void PollingTimerStopInit(void);

// for Func sw
 static int irq_func=0;
 static unsigned int func_pushed_status;
 #define FUNC_IRQ (IRQ_GPP_START + BIT_FUNC_SW)

 #define FUNC_POL_MSEC		(HZ / POLLING_COUNTS_PER_1SEC)
 #define FUNC_WAIT_SECOND	(1 * HZ)
 #define FUNC_WAIT_COUNT	(0)
 #define FUNC_LONG_WAIT_COUNT	(FUNC_WAIT_SECOND / FUNC_POL_MSEC)

 static struct timer_list FuncPollingTimer;
 static void PollingTimerGoOnFunc(void);
 static void PollingTimerStopFunc(void);


static void PollingPowerSWStatus(unsigned long data)
{
	TRACE(printk(">%s, data=%u, PSW_LONG_WAIT_COUNT=%d\n", __FUNCTION__, data, PSW_LONG_WAIT_COUNT));

	if (bfGppInRegBitTest(BIT_PWR_SW)) {
		if((data >= PSW_LONG_WAIT_COUNT) && !(power_pushed_status & SW_LONG_PUSHED)){
			power_pushed_status |= SW_LONG_PUSHED;
#if defined CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER
			buffalo_kernevnt_queuein("PSW_pushed");
#endif
		}else if((data >= PSW_WAIT_COUNT) && !(power_pushed_status & SW_PUSHED)){
			power_pushed_status |= SW_PUSHED;
#if defined CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER
			buffalo_kernevnt_queuein("PSW_short_pushed");
#endif
		}
		PollingTimerGoOn();
	}else{
		PollingTimerStop();
		if(g_irq)
			enable_irq(g_irq);

		power_pushed_status = 0;
	}

	return;
}

static void PollingTimerStop(void)
{
	del_timer(&PSWPollingTimer);
}

static void PollingTimerGoOn(void)
{
	PSWPollingTimer.expires=(jiffies + PSW_POL_MSEC);
	PSWPollingTimer.data+=1;
	add_timer(&PSWPollingTimer);
}

static void PollingTimerStart(void)
{
	init_timer(&PSWPollingTimer);
	PSWPollingTimer.expires=(jiffies + PSW_POL_MSEC);
	PSWPollingTimer.function=PollingPowerSWStatus;
	PSWPollingTimer.data=0;
	add_timer(&PSWPollingTimer);
}

static int PowerSWInt_en_proc_show(struct seq_file *m, void *v)
{

	seq_printf(m, "PowerIntAct\n");

	if(g_irq){
		disable_irq(g_irq);
		enable_irq(g_irq);
	}

	return 0;
}

static int PowerSWInt_en_proc_open(struct inode *inode, struct file *file)
{

	return single_open(file, PowerSWInt_en_proc_show, NULL);
}

static const struct file_operations buffalo_PowerSWInt_en_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= PowerSWInt_en_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};


static irqreturn_t PowSwInterrupts(int irq, void *dev_id)
{
	TRACE(printk(">%s\n", __FUNCTION__));
	//Process to checking power sw pushed or not.

	disable_irq_nosync(irq);
	g_irq=irq;

	// Setup PollingPowerSWStatus;
	PollingTimerStart();
	TRACE(printk(">%s\n", __FUNCTION__));

	return IRQ_HANDLED;
}



static void
PollingINITSWStatus(unsigned long data)
{
	
	TRACE(printk(">%s, data=%u, INIT_WAIT_COUNT=%d\n", __FUNCTION__, data, INIT_WAIT_COUNT));

	if (bfGppInRegBitTest(BIT_INIT_SW)) {
		if((data > INIT_WAIT_COUNT) && !(init_pushed_status & SW_PUSHED)){
			init_pushed_status |= SW_PUSHED;
#if defined CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER
			buffalo_kernevnt_queuein("INITSW_pushed");
#else

#endif
			// keep Polling untile button would be released.
			TRACE(printk("initialize start\n"));
		}
			PollingTimerGoOnInit();
	}else{
		PollingTimerStopInit();
		if(g_irq_init)
			enable_irq(g_irq_init);
	}

	return;
}


static void
PollingTimerStopInit(void)
{
	del_timer(&INITPollingTimer);
}


static void
PollingTimerGoOnInit(void)
{
	INITPollingTimer.expires=(jiffies + INIT_POL_MSEC);
	INITPollingTimer.data+=1;
	add_timer(&INITPollingTimer);
}


static void
PollingTimerStartInit(void)
{
	init_timer(&INITPollingTimer);
	INITPollingTimer.expires=(jiffies + INIT_POL_MSEC);
	INITPollingTimer.function=PollingINITSWStatus;
	INITPollingTimer.data=0;
	add_timer(&INITPollingTimer);
}

static irqreturn_t
InitSwInterrupts(int irq, void *dev_id)
{
	TRACE(printk(">%s\n", __FUNCTION__));

	disable_irq_nosync(irq);
	g_irq_init=irq;

	PollingTimerStartInit();
	TRACE(printk(">%s\n", __FUNCTION__));

	return IRQ_HANDLED;
}



static void
PollingFuncSWStatus(unsigned long data)
{
	TRACE(printk(">%s, data=%u, FUNC_WAIT_COUNT=%d\n", __FUNCTION__, data, FUNC_WAIT_COUNT));

	if (bfGppInRegBitTest(BIT_FUNC_SW)) {
		func_pushed_status |= SW_PUSHED;
		if ((data > FUNC_LONG_WAIT_COUNT) && !(func_pushed_status & SW_LONG_PUSHED)){
			func_pushed_status |= SW_LONG_PUSHED;
#if defined CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER
			buffalo_kernevnt_queuein("FUNCSW_long_pushed");
#endif
		}
		PollingTimerGoOnFunc();
	}
	else {
		PollingTimerStopFunc();
		if (irq_func)
			enable_irq(irq_func);
		
		if ((data > FUNC_WAIT_COUNT) && !(func_pushed_status ^ SW_PUSHED)) {
#if defined CONFIG_BUFFALO_USE_KERNEL_EVENT_DRIVER
			buffalo_kernevnt_queuein("FUNCSW_pushed");
#endif
		}
		func_pushed_status = 0;
	}

	return;
}

static void
PollingTimerStopFunc(void)
{
	del_timer(&FuncPollingTimer);
}

static void
PollingTimerGoOnFunc(void)
{
	FuncPollingTimer.expires=(jiffies + FUNC_POL_MSEC);
	FuncPollingTimer.data+=1;
	add_timer(&FuncPollingTimer);
}

static void
PollingTimerStartFunc(void)
{
	init_timer(&FuncPollingTimer);
	FuncPollingTimer.expires=(jiffies + FUNC_POL_MSEC);
	FuncPollingTimer.function=PollingFuncSWStatus;
	FuncPollingTimer.data=0;
	add_timer(&FuncPollingTimer);
}

static irqreturn_t
FuncSwInterrupts(int irq, void *dev_id)
{
	TRACE(printk(">%s\n", __FUNCTION__));

	disable_irq_nosync(irq);
	irq_func=irq;

	PollingTimerStartFunc();
	TRACE(printk(">%s\n", __FUNCTION__));

	return IRQ_HANDLED;
}


/*
 * Initialize driver.
 */
int __init BuffaloCpuInterrupts_init(void)
{
	int rv;						/* avoid __must_check */
	TRACE(printk(">%s\n", __FUNCTION__));

	if (!use_slide_power && BIT_PWR_SW >= 0) {
		proc_create("PowerSWInt_en", 0, get_proc_buffalo(), &buffalo_PowerSWInt_en_proc_fops);

		//		BuffaloGpio_CPUInterruptsSetup();
		rv = request_irq(PSW_IRQ, PowSwInterrupts, 0, "PowSw", NULL);
	}

	if (BIT_INIT_SW >= 0) {
	  //		BuffaloGpio_CPUInterruptsSetupInit();
		rv = request_irq(INIT_IRQ, InitSwInterrupts, 0, "InitSw", NULL);
	}

	if (BIT_FUNC_SW >= 0) {
	  //		BuffaloGpio_CPUInterruptsSetupFunc();
		rv = request_irq(FUNC_IRQ, FuncSwInterrupts, 0, "FuncSw", NULL);
	}

	printk("%s %s Ver.%s installed.\n", DRIVER_NAME, AUTHOR, BUFFALO_DRIVER_VER);
	return 0;
}

void BuffaloCpuInterrupts_exit(void)
{
	TRACE(printk(">%s\n", __FUNCTION__));

	if (!use_slide_power) {
		free_irq(PSW_IRQ, NULL);
		PollingTimerStop();
	}

	remove_proc_entry("PowerSWInt_en", get_proc_buffalo());
	printk("%s %s uninstalled.\n", DRIVER_NAME, BUFFALO_DRIVER_VER);
}

module_init(BuffaloCpuInterrupts_init);
module_exit(BuffaloCpuInterrupts_exit);

