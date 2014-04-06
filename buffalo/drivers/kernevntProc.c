/*
 *  LinkStation/TeraStation Kernel Event Proc Driver
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
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19) */
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19)
#include <linux/sched.h>
#endif /* LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19) */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/string.h>

#if defined(CONFIG_ARCH_FEROCEON_KW)
#include "buffalocore.h"
#include "kernevntProc.h"
#else
#include "buffalocore.h"
#include "kernevntProc.h"
#endif
#include "buffalo_proc_entry.h"

#define bzero(p,sz) memset(p,0,sz)

//#define DEBUG

#ifdef DEBUG
 #define TRACE(x) x
#else
 #define TRACE(x)
#endif

extern void buffalo_kernevnt_queuein(const char *cmd);
extern int buffalo_kernevnt_queueout(unsigned char *cmd, int *len);
extern wait_queue_head_t  buffalo_kernevnt_WaitQueue;




// event notice from kernel
static int BuffaloKernevnt_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	char retbuff[MAX_CMDLEN+1+2];
	
	TRACE(printk(">%s count=%d off=%ld\n",__FUNCTION__,count,off));
	if (count < 0)
		return -EINVAL;
	
	if (off>0 || wait_event_interruptible(buffalo_kernevnt_WaitQueue, buffalo_kernevnt_queueout(retbuff,&len)>=0 )!=0){
		*eof = 1;
		return 0;
	}
	
	if (len>count){
		// buffer too short
		return -EINVAL;
	}
	*eof = 1;
	memcpy(page,retbuff,len);
	//printk("len=%d eof=%d off=%ld count=%d [%s]\n",len,*eof,off,count,retbuff);
	
	return len;
}



/*
 * Initialize driver.
 */
int BuffaloKernevnt_init (void)
{
	TRACE(printk(">%s\n",__FUNCTION__));
	printk("Kernel event proc (C) BUFFALO INC. V.1.00 installed.\n"); 
	
	create_proc_read_entry("kernevnt", 0, get_proc_buffalo(), BuffaloKernevnt_read_proc, NULL);
	
	return 0;
}

void BuffaloKernevnt_exit(void)
{
	TRACE(printk(">%s\n",__FUNCTION__));
	remove_proc_entry("kernevnt", get_proc_buffalo());
	printk("Kernel event proc removed.\n");
}


#ifdef CONFIG_X86
module_init(BuffaloKernevnt_init);
module_exit(BuffaloKernevnt_exit);
#endif

MODULE_LICENSE("GPL");

