/*
 *  LinkStation/TeraStation Kernel Event Driver
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
#include <linux/sched.h>
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#include <asm/string.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/serial.h>
#include "buffalo/kernevnt.h"
#if defined(CONFIG_ARCH_FEROCEON_KW)
#include "buffalo/kernevntProc.h"
#include "mv_network/mv_ethernet/mv_netdev.h"
#else
#include "kernevntProc.h"
#endif

#define bzero(p,sz) memset(p,0,sz)

//#define DEBUG
//#define FUNCMSG

#ifdef DEBUG
 #define TRACE(x) x
#else
 #define TRACE(x)
#endif

static spinlock_t BuffaloMicon_event_lock = SPIN_LOCK_UNLOCKED;
wait_queue_head_t buffalo_kernevnt_WaitQueue;
static int initialized=0;
static struct tag_MiconDevice {
	struct {
		char cmd[MAX_CMDLEN+1];
	} cmdqueue[MAX_QUELEN];
	int rindex,windex;
} MiconDevice;

//--------------------------------------------------------------
void buffalo_kernevnt_queuein(const char *cmd)
{
	if (!initialized){
		return;
	}
	if (MiconDevice.cmdqueue[MiconDevice.windex].cmd[0]==0){
		
		spin_lock_irq (&BuffaloMicon_event_lock);
		
		strncpy(MiconDevice.cmdqueue[MiconDevice.windex].cmd, cmd, MAX_CMDLEN-1);
		MiconDevice.windex++;
		if (MiconDevice.windex == sizeof(MiconDevice.cmdqueue)/sizeof(MiconDevice.cmdqueue[0])){
			MiconDevice.windex=0;
		}
		
		spin_unlock_irq (&BuffaloMicon_event_lock);
		
		TRACE(printk("queue in : r=%d w=%d [%s]\n",MiconDevice.rindex,MiconDevice.windex, cmd));
	}else{
		TRACE(printk("queue full [%s]\n",cmd));
	}
	wake_up_interruptible(&buffalo_kernevnt_WaitQueue);
}

//--------------------------------------------------------------
int buffalo_kernevnt_queueout(unsigned char *cmd, int *len)
{
	if (!initialized){
		return -1;
	}
	if (MiconDevice.cmdqueue[MiconDevice.rindex].cmd[0]!=0){
		int index;
		index = MiconDevice.rindex;
		
		TRACE(printk("queue out : r=%d w=%d [%s]\n",MiconDevice.rindex,MiconDevice.windex, MiconDevice.cmdqueue[index].cmd));
		
		spin_lock_irq (&BuffaloMicon_event_lock);
		
		if (MiconDevice.cmdqueue[index].cmd[0]!=0){
			strncpy(cmd, MiconDevice.cmdqueue[index].cmd,MAX_CMDLEN);
			*len = strlen(cmd)+1;  // for NULL terminate.
			MiconDevice.cmdqueue[index].cmd[0]=0;
		}
		MiconDevice.rindex++;
		if (MiconDevice.rindex == sizeof(MiconDevice.cmdqueue)/sizeof(MiconDevice.cmdqueue[0])){
			MiconDevice.rindex=0;
		}
		
		spin_unlock_irq (&BuffaloMicon_event_lock);
		
		return index;
	}else{
		TRACE(printk("queue none\n"));
	}
	return -1;
}

#ifdef CONFIG_MD
//--------------------------------------------------------------
void kernevnt_RaidRecovery(int devno, int on, int isRecovery, int major, int minor)
{
	static int opencnt=0;
	char buff[64];
#ifdef FUNCMSG
	printk(">%s:md%d on=%d cnt=%d isRecovery=%d %d %d\n",__FUNCTION__,devno,on,opencnt,isRecovery,major,minor);
#endif
	
	if (on) opencnt++;
	else    opencnt--;
	
	if (isRecovery){
		sprintf(buff,"raidrecovery %d %d %d %d %d %d",devno,on,opencnt,isRecovery,major,minor);
	}else{
		sprintf(buff,"raidresync %d %d %d %d",devno,on,opencnt,isRecovery);
	}
	buffalo_kernevnt_queuein(buff);
}
//--------------------------------------------------------------
void kernevnt_RaidScan(int devno, int on)
{
	static int opencnt=0;
	char buff[64];
#ifdef FUNCMSG
	printk(">%s:md%d on=%d cnt=%d\n",__FUNCTION__,devno,on,opencnt);
#endif
	
	if (on) opencnt++;
	else    opencnt--;
	
	sprintf(buff,"raidscan %d %d %d",devno,on,opencnt);
	buffalo_kernevnt_queuein(buff);
}
//--------------------------------------------------------------
void kernevnt_RaidDegraded(int devno, int major, int minor)
{
	char buff[64];
#ifdef FUNCMSG
	printk(">%s:md%d %d %d\n",__FUNCTION__,devno,major,minor);
#endif
	
	sprintf(buff,"raiddegraded %d %d %d",devno,major,minor);
	buffalo_kernevnt_queuein(buff);
}
//--------------------------------------------------------------
void kernevnt_RaidReshape(int devno, int on)
{
	static int opencnt=0;
	char buff[64];

	if (on) opencnt++;
	else    opencnt--;

	sprintf(buff, "raidreshape %d %d %d", devno, on, opencnt);
	buffalo_kernevnt_queuein(buff);
}

#endif //#ifdef CONFIG_MD

//--------------------------------------------------------------
void kernevnt_FlashUpdate(int on)
{
#ifdef FUNCMSG
	printk(">%s (%d)\n",__FUNCTION__,on);
#endif
	char msg[32];
	sprintf(msg,"flashupdate %d",on);
	buffalo_kernevnt_queuein(msg);
}

//--------------------------------------------------------------
void kernevnt_LanAct(void *data)
{
#if defined(CONFIG_ARCH_FEROCEON_KW)
	mv_eth_priv *priv = (mv_eth_priv *)data;
	char msg[32];
	int port = priv->port;
	u32 port_status;
	int linkspeed;

	port_status = MV_REG_READ(ETH_PORT_STATUS_REG(port));
	if (port_status & ETH_LINK_UP_MASK) {
		if(port_status & ETH_GMII_SPEED_1000_MASK) {
			linkspeed = 1000;
		}
		else {
			linkspeed = ((port_status & ETH_MII_SPEED_100_MASK) ? 100 : 10 );
		}
	}
	else {
		linkspeed = 0;
	}

	sprintf(msg,"lanact %d %s eth%d",
		linkspeed,
		(port_status & ETH_FULL_DUPLEX_MASK) ? "full" : "half",
		port);
	
	buffalo_kernevnt_queuein(msg);
#endif
}

void kernevnt_SwitchHubAct(const char *msg)
{
	buffalo_kernevnt_queuein(msg);
}

//--------------------------------------------------------------
void kernevnt_IOErr(const char *kdevname, const char *dir, unsigned long sector, unsigned int errcnt)
{
#ifdef FUNCMSG
		printk(">%s(%s): I/O error, dev %s, sector %lu\n",
			__FUNCTION__,kdevname, dir, sector, errcnt);
#endif
	char msg[64];
	sprintf(msg,"ioerr %s %s %ld %u",kdevname, dir, sector, errcnt);
	buffalo_kernevnt_queuein(msg);
}

//--------------------------------------------------------------
void kernevnt_DriveDead(const char *drvname)
{
#ifdef FUNCMSG
		printk(">%s: drive %s is dead.\n",__FUNCTION__,drvname);
#endif
	char msg[64];
	sprintf(msg,"drivedead %s",drvname);
	buffalo_kernevnt_queuein(msg);
}

//--------------------------------------------------------------
void kernevnt_I2cErr(void)
{
	printk(">%s\n",__FUNCTION__);
	buffalo_kernevnt_queuein("i2c_error");
}

//--------------------------------------------------------------
void kernevnt_MiconInt(void)
{
#ifdef FUNCMSG
		printk(">%s\n",__FUNCTION__);
#endif
	buffalo_kernevnt_queuein("micon_interrupts");
}

//--------------------------------------------------------------
void kernevnt_EnetOverload(const char *name)
{
#ifdef FUNCMSG
		printk(">%s\n",__FUNCTION__);
#endif
	char msg[64];
	sprintf(msg,"enetover %s",name);
	buffalo_kernevnt_queuein(msg);
}

//--------------------------------------------------------------
/*
 * Initialize driver.
 */
int __init BuffaloMicon_init (void)
{
	TRACE(printk(">%s\n",__FUNCTION__));
	printk("MICON ctrl (C) BUFFALO INC. V.1.00 installed.\n"); 
	
	// Initialize Local valiables
	bzero(&MiconDevice, sizeof(MiconDevice));
	init_waitqueue_head(&buffalo_kernevnt_WaitQueue);
	initialized=1;
	return 0;
}

//--------------------------------------------------------------
void BuffaloMicon_exit(void)
{
	TRACE(printk(">%s\n",__FUNCTION__));
	printk("MICON ctrl removed.\n");
	initialized=0;

}

EXPORT_SYMBOL(buffalo_kernevnt_queuein);
EXPORT_SYMBOL(buffalo_kernevnt_queueout);
EXPORT_SYMBOL(buffalo_kernevnt_WaitQueue);

module_init(BuffaloMicon_init);
module_exit(BuffaloMicon_exit);
MODULE_LICENSE("GPL");

