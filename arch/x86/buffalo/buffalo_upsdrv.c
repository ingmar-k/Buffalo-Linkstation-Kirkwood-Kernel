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

#include <linux/serial_reg.h>
#include <linux/serial_core.h>
#include <linux/serial_reg.h>

#define UPSPORT	0
#if defined(CONFIG_BUFFALO_USE_UPS)

struct uart_port *uart_ports[CONFIG_SERIAL_8250_NR_UARTS];

#define BUFFALO_SUPPORT_OMRON_UPS_SHUTDOWN

static struct proc_dir_entry *proc_buffalo_ups;
static unsigned int OmronShutdownEnable = 0;

extern struct uart_port *uart_ports[];

static int BuffaloUps_OmronShutdownEnable_read_proc(char *page, char **start, off_t offset, int length)
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
//--------------------------------------------------------------
/*
 * Initialize driver.
 */
int __init BuffaloUpsdrv_init (void)
{
	printk("UPSDRV (C) BUFFALO INC. V.1.00 installed.\n"); 
	
	proc_buffalo_ups = proc_mkdir("buffalo/ups", 0);

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

#if defined(BUFFALO_SUPPORT_OMRON_UPS_SHUTDOWN)
	remove_proc_entry("buffalo/ups/omron_shutdown_enable", 0);
#endif	// of BUFFALO_SUPPORT_OMRON_UPS_SHUTDOWN
	remove_proc_entry("buffalo/ups", 0);
}

module_init(BuffaloUpsdrv_init);
module_exit(BuffaloUpsdrv_exit);
MODULE_LICENSE("GPL");

#endif  //CONFIG_BUFFALO_USE_UPS
