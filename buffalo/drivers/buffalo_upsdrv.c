/*
 *  LinkStation/TeraStation UPS port Driver
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
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
#include <linux/config.h>
#endif /* LINUX_VERSION_CODE < KERNEL_VERSIOIN(2,6,19) */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>

#ifdef CONFIG_X86
#include <linux/serial_reg.h>
#include <linux/serial_core.h>
#include <linux/serial_reg.h>

#define UPSPORT	0
#endif // CONFIG_X86

#if defined(CONFIG_BUFFALO_USE_UPS)

#ifdef CONFIG_X86
struct uart_port *uart_ports[CONFIG_SERIAL_8250_NR_UARTS];
#else // CONFIG_X86
#include "buffalocore.h"
#include "BuffaloGpio.h"

#define bzero(p,sz) memset(p,0,sz)

#define USE_UPS_RECOVER

//#define DEBUG_UPS

/* Function prototypes */


/* variables */
static struct proc_dir_entry *proc_buffalo_ups;

#ifdef DEBUG_UPS
//----------------------------------------------------------------------
static int BuffaloMppConfig_read_proc(char *page, char **start, off_t offset, int length)
{
	//int              i;
	int              len  = 0;
	char*            buf  = page;
	off_t begin = 0;

	unsigned base=0xF1000000;
	volatile unsigned int *mpp_reg=(unsigned *)(base+0x10000);
	
	len += sprintf(buf, "Dumped base = 0x10000\noffset 0\t%x\noffset 4\t%x\noffset 8\t%x\n",
		mpp_reg[0], mpp_reg[1], mpp_reg[2]);
	
	*start = page + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return (len);
}

#endif
//----------------------------------------------------------------------

#if defined USE_UPS_RECOVER
//--------------------------------------------------------------
static void BuffaloUpsRecoverInit(void){
	
	BuffaloRtc_UPSRecoverInit();
	printk("BUFFALO UPS Recover Function Initialized.\n");

}

//--------------------------------------------------------------
static int BuffaloUpsRecoverWriteProc(struct file *filep, const char *buffer, unsigned long count, void *data){
	int8_t OnTarget=0;	

	if(strncmp(buffer, "apc_on", 6) == 0){
		OnTarget = RECOVER_TARGET_UPS_APC;	
	}else if(strncmp(buffer, "omron_on", 8) == 0){
		OnTarget = RECOVER_TARGET_UPS_OMR;
//		printk("%s %s>on entered\n", __FILE__, __FUNCTION__);
	}else if(strncmp(buffer, "usb_on", 6) == 0){
		OnTarget = RECOVER_TARGET_UPS_USB;
	}else if(strncmp(buffer, "off", 3) ==0){
//		printk("%s %s>off entered\n", __FILE__, __FUNCTION__);
		BuffaloRtc_UPSRecoverDisable();
	}
	if(OnTarget){
		BuffaloRtc_UPSRecoverEnable(OnTarget);
	}
	return count;
}

//--------------------------------------------------------------
static int BuffaloUpsRecoverReadProc(char *page, char **start, off_t offset, int length, int *eof, void *data){
	
	int len = 0;
	int ret = BuffaloRtc_UPSRecoverReadStatus();
	
	switch(ret){
	case RECOVER_TARGET_UPS_APC:
		len = sprintf(page, "apc_on\n");
		break;
	case RECOVER_TARGET_UPS_OMR:
		len = sprintf(page, "omron_on\n");
		break;
	case RECOVER_TARGET_UPS_USB:
		len = sprintf(page, "usb_on\n");
		break;
	default:
		len = sprintf(page, "off\n");
		break;
	}
	
	*start = page + offset;
	len -= offset;
	
	if(len > length)
		len=length;
	
	return (len);
}
#endif // end of USE_UPS_RECOVER
#endif // CONFIG_X86

#if !defined(CONFIG_ARCH_FEROCEON_KW)
#define BUFFALO_SUPPORT_OMRON_UPS_SHUTDOWN

#ifdef CONFIG_X86
static struct proc_dir_entry *proc_buffalo_ups;
#else // CONFIG_X86
#include "BuffaloUart.h"
#include <linux/serial_core.h>
#endif // CONFIG_X86
static unsigned int OmronShutdownEnable = 0;

extern struct uart_port *uart_ports[];

#ifdef CONFIG_X86
static int BuffaloUps_OmronShutdownEnable_read_proc(char *page, char **start, off_t offset, int length)
#else // CONFIG_X86
static int BuffaloUps_OmronShutdownEnable_read_proc(char *page, char **start, off_t offset, int length, int *eof, void *data)
#endif // CONFIG_X86
{
	int len = 0;

	if(OmronShutdownEnable)
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");

	*start = page + offset;
	len -= offset;

	if(len > length)
		len = length;

	return len;
}

static int BuffaloUps_OmronShutdownEnable_write_proc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if(strncmp(buffer, "on", 2) == 0)
		OmronShutdownEnable = 1;
	else if(strncmp(buffer, "off", 3) == 0)
		OmronShutdownEnable = 0;

	return count;
}

void BuffaloUps_ShutdownUps(void)
{
	int timeout = 15;
	int i = 0;

	if(OmronShutdownEnable == 1)
	{
		unsigned int mctrl = uart_ports[UPSPORT]->ops->get_mctrl(uart_ports[UPSPORT]);
		printk("Shuttting down OMRON ups ... \n");

#if defined(CONFIG_ARCH_FEROCEON_KW) || defined(CONFIG_ARCH_FEROCEON_MV78XX0)
		mctrl &= ~TIOCM_DTR;	// ARMの場合は、TIOCM_DTRが論理反転していたため。
#else
		mctrl |= TIOCM_DTR;
#endif
		uart_ports[UPSPORT]->ops->set_mctrl(uart_ports[UPSPORT], mctrl);

		while(timeout)
		{
#if 0
			// force shutdown UPS output and recover.
			mctrl = uart_ports[UPSPORT]->ops->get_mctrl(uart_ports[UPSPORT]);
			if(!(mctrl & TIOCM_CTS))
			{
				printk("AC recovered. Exiting shutting down loop!\n");
				break;
			}
#endif
			for(i = 0; i < 1000; i++)
			{
				udelay(1000);
			}
			timeout--;
		}
		if(timeout == 0)
			printk("Failed?\n");
	}
}
#endif // !defined(CONFIG_ARCH_FEROCEON_KW)

//--------------------------------------------------------------
/*
 * Initialize driver.
 */
int __init BuffaloUpsdrv_init (void)
{
#ifdef CONFIG_X86
#else // CONFIG_X86
#if defined USE_UPS_RECOVER
	struct proc_dir_entry *generic_ups_recover;
#endif
#endif // CONFIG_X86
	printk("UPSDRV (C) BUFFALO INC. V.1.00 installed.\n"); 
	
	proc_buffalo_ups = proc_mkdir("buffalo/ups", 0);
#ifdef CONFIG_X86
#else // CONFIG_X86
#if defined USE_UPS_RECOVER
	BuffaloUpsRecoverInit();
	generic_ups_recover = create_proc_entry("ups_recover", 0, proc_buffalo_ups);
	generic_ups_recover->read_proc=&BuffaloUpsRecoverReadProc;
	generic_ups_recover->write_proc=&BuffaloUpsRecoverWriteProc;
#endif
#endif // CONFIG_X86

#if defined(BUFFALO_SUPPORT_OMRON_UPS_SHUTDOWN)
	struct proc_dir_entry *omr_std_en;

	omr_std_en = create_proc_entry("buffalo/ups/omron_shutdown_enable", 0, 0);
	omr_std_en->read_proc = &BuffaloUps_OmronShutdownEnable_read_proc;
	omr_std_en->write_proc =&BuffaloUps_OmronShutdownEnable_write_proc;
	printk("  OMRON contact type ups shutdown support enabled!\n");
#endif	// of BUFFALO_SUPPORT_OMRON_UPS_SHUTDOWN

	return 0;
}

//--------------------------------------------------------------
void BuffaloUpsdrv_exit(void)
{
	printk("UPSDRV removed.");

#ifdef CONFIG_X86
#else // CONFIG_X86
#if defined USE_UPS_RECOVER
	remove_proc_entry("ups_recover", proc_buffalo_ups);
	proc_buffalo_ups = NULL;
#endif
#endif // CONFIG_X86
#if defined(BUFFALO_SUPPORT_OMRON_UPS_SHUTDOWN)
	remove_proc_entry("buffalo/ups/omron_shutdown_enable", 0);
#endif	// of BUFFALO_SUPPORT_OMRON_UPS_SHUTDOWN
#ifdef CONFIG_X86
#else // CONFIG_X86
	proc_buffalo_ups = NULL;
#endif // CONFIG_X86
	remove_proc_entry("buffalo/ups", 0);
}

module_init(BuffaloUpsdrv_init);
module_exit(BuffaloUpsdrv_exit);
MODULE_LICENSE("GPL");

#endif  //CONFIG_BUFFALO_USE_UPS
