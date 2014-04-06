/*
 *  LinkStation/TeraStation Micon port Driver
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
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>

#include "boardEnv/mvBoardEnvLib.h"

#include "gpp/mvGpp.h"
#include "gpp/mvGppRegs.h"

#include "buffalocore.h"
#include "miconcntl.h"
#include "buffalo/kernevnt.h"
#include "BuffaloUart.h"
#include "BuffaloGpio.h"
#include "buffalo_proc_entry.h"

#if defined(CONFIG_BUFFALO_LINKSTATION_LSGL) || 1
 #ifdef CONFIG_CPU_ARM926T
    // see. 88F5182 User Manual : A.4.3. Main Interrupt Controller Registers
    // BIT0 : irq 32
    // BIT2 : Micon
    // BIT3 : RTC
    //
    #define MICON_IRQ (32+BIT_MICON)
 #endif
#elif defined (CONFIG_BUFFALO_TERASTATION_TSTGL)
  #define MICON_IRQ 18
#elif defined (CONFIG_BUFFALO_TERASTATION_TSHTGL)
  #define MICON_IRQ (32+BIT_MICON)
#elif defined (CONFIG_BUFFALO_LINKSTATION_HSWDHTGL_R1)
  #define MICON_IRQ (32+BIT_MICON)
#else
 #error
#endif

#define bzero(p,sz) memset(p,0,sz)

//#define DEBUG
//#define MICONMSG


#ifdef DEBUG
 #define TRACE(x) x
#else
 #define TRACE(x)
#endif

static int g_irq=0;
static int initialized = 0;

//----------------------------------------------------------------------
static int MiconIntActivate_proc_show(struct seq_file *m, void *v)
{

	seq_printf(m, "MIntAct\n");

	if (g_irq != 0)
		enable_irq(g_irq);	// to enable LEVEL-sensitive interrupt.
	return 0;
}

static int MiconIntActivate_proc_open(struct inode *inode, struct file *file)
{

	return single_open(file, MiconIntActivate_proc_show, NULL);
}

static const struct file_operations buffalo_MiconIntActivate_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= MiconIntActivate_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#ifdef MICONMSG
//--------------------------------------------------------------
static void dumpdata(const char *title,const unsigned char *data, int len)
{
	int i;

	printk("--- %s %d ---\n",title,len);
	for (i=0; i<len; i++){
		printk("%02x ",data[i]);
	}
	printk("\n");
}

#endif

//--------------------------------------------------------------
//ppc only
static int MiconPortWrite(const unsigned char *buf, int count)
{
#ifdef MICONMSG
	printk(">%s:count=%d\n",__FUNCTION__,count);
	dumpdata(__FUNCTION__,buf,count);
#endif

	BuffaloMiconOutput(buf ,count);
	return 0;
}

//--------------------------------------------------------------
//ppc only
static int MiconPortRead(unsigned char *buf, int count)
{
	int i;

#ifdef MICONMSG
	printk(">%s\n",__FUNCTION__);
#endif

	for (i=0; i<count; i++){
		if (BuffaloMiconInput(buf+i,100)!=0){
			break;
		}
	}

#ifdef MICONMSG
	printk(">%s:count=%d\n",__FUNCTION__,i);
	dumpdata(__FUNCTION__,buf,i);
#endif
	return i;		// return read bytes
}

//--------------------------------------------------------------
static void miconCntl_SendPreamble(void)
{
	unsigned char buff[40];
#ifdef MICONMSG
	printk(">%s\n",__FUNCTION__);
#endif
	memset(&buff,0xff,sizeof(buff));
	MiconPortWrite(buff,sizeof(buff));
	mdelay(100);
	// make dummy read.
	MiconPortRead(buff,sizeof(buff));
	mdelay(100);
	MiconPortRead(buff,sizeof(buff));
	mdelay(100);
	MiconPortRead(buff,sizeof(buff));
}

//--------------------------------------------------------------
static int miconCntl_SendCmd(const unsigned char *data, int count)
{
	int i;
	unsigned char parity;
	unsigned char recv_buf[35];
	int retry=2;

	TRACE(printk(">%s\n",__FUNCTION__));

	//Generate checksum
	parity = 0;
	for(i=0;i<count;i++){
		parity +=  data[i];
	}
	parity = 0 - parity ;

	mdelay(10);		// interval for next command

	do {
#ifdef MICONMSG
		printk(">%s retry=%d\n",__FUNCTION__,retry);
#endif
		// send data
		MiconPortWrite(data,count);
		// send parity
		MiconPortWrite(&parity,1);
		if (MiconPortRead(recv_buf,sizeof(recv_buf))<=3){
			printk(">%s: recv fail.\n",__FUNCTION__);
			// send preamble
			miconCntl_SendPreamble();
		}else{
			//Generate Recive data
			unsigned char correct_ACK[4];
			correct_ACK[0] = 0x01;
			correct_ACK[1] = data[1];
			correct_ACK[2] = 0x00;
			correct_ACK[3] = 0 - (0x01 + data[1] + 0x00);

			//Parity Check
			if(0 != (0xFF & (recv_buf[0] + recv_buf[1] + recv_buf[2] + recv_buf[3]))){
				printk("Parity Error : Recive data[%02x, %02x, %02x, %02x]\n",
						recv_buf[0], recv_buf[1], recv_buf[2], recv_buf[3]);
			}else{
				//Check Recive Data
				if( (correct_ACK[0] == recv_buf[0]) && (correct_ACK[1] == recv_buf[1])
						&& (correct_ACK[2] == recv_buf[2]) && (correct_ACK[3] == recv_buf[3])){
					//Recive ACK
					return 0;
				}
			}
			//Recive NAK or illegal Data
			printk("Error : NAK or Illegal Data Recived\n");
		}
	} while(retry--);
	return -1;
}


//--------------------------------------------------------------
static void miconCntl_ShutdownWait(void)
{
	const unsigned char WDkill_msg[] = {0x01,0x35,0x00};
	const unsigned char SdWait[] = {0x00,0x0c};
	const unsigned char BootEnd[] = {0x00,0x03};

	printk(">%s\n",__FUNCTION__);

	while(miconCntl_SendCmd(WDkill_msg,sizeof(WDkill_msg))) ;
	while(miconCntl_SendCmd(BootEnd,sizeof(BootEnd))) ;
	while(miconCntl_SendCmd(SdWait,sizeof(SdWait))) ;
}

//--------------------------------------------------------------
void miconCntl_Reboot(void)
{
	const unsigned char reboot_msg[] = {0x00,0x0E};

	if (!initialized)
		return;
	printk(">%s\n",__FUNCTION__);

	BuffaloInitUart();
	miconCntl_ShutdownWait();

#ifdef MICONMSG
	printk("Send Reboot\n");
#endif
	//Send Reboot CMD
	// halt
	while (1){
		miconCntl_SendCmd(reboot_msg,sizeof(reboot_msg));
		mdelay(4000);
	}
}

//--------------------------------------------------------------
void miconCntl_PowerOff(void)
{
	const unsigned char poff_msg[] = {0x00,0x06};

	if (!initialized)
		return;
	printk(">%s\n",__FUNCTION__);

	BuffaloInitUart();
	miconCntl_ShutdownWait();

#ifdef MICONMSG
	printk("Send Poweroff\n");
#endif
	//Send Poweroff CMD
	// halt
	while (1){
		miconCntl_SendCmd(poff_msg,sizeof(poff_msg));
		mdelay(4000);
	}
}

//--------------------------------------------------------------
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static int micon_interrupts(int irq, void *dev_id, struct pt_regs *regs)
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19) */
static irqreturn_t micon_interrupts(int irq, void *dev_id)
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19) */
{
	//printk(">%s ***** \n",__FUNCTION__);

	//BuffaloPrintGpio();	/* arch/arm/mach-*/Board/buffalo/BuffaloGpio.c */
	kernevnt_MiconInt();
    /* Clear cause register */
	BuffaloGpio_ClearMiconInt();

	// edge sensitive is not available in ARM platform.
	// so, disable interrupt in this time.
	// you can re-enable this interrupt by reading /proc/buffalo/miconint_en file.

	disable_irq_nosync(irq);
	g_irq = irq;

	return IRQ_HANDLED;
}

//--------------------------------------------------------------
/*
 * Initialize driver.
 */
int __init BuffaloMiconV2_init (void)
{
	//struct proc_dir_entry *generic;
/*
~ # cat /proc/interrupts
           CPU0
  0:       2984   Mv Timer Tick
  3:        378   serial
 12:          0   ehci_hcd:usb2
 17:          0   ehci_hcd:usb1
 21:         12   eth0
 28:          0   cesa
 29:          0   mvSata
 34:          0   MiCon
Err:          0
*/
	MV_U32 bit_micon_irq;
	int rv = -1;

	if(mvBoardGpioPinNumGet(BOARD_GPP_MC_IRQ, 0) == -1)
		return 0;

	TRACE(printk(">%s\n",__FUNCTION__));
	bit_micon_irq = IRQ_GPP_START + mvBoardGpioPinNumGet(BOARD_GPP_MC_IRQ, 0);
	printk("MICON V2 (C) BUFFALO INC. V.2.00 installed.(IRQ=%d)\n", bit_micon_irq);

	proc_create("miconint_en", 0, get_proc_buffalo(), &buffalo_MiconIntActivate_proc_fops);

	BuffaloInitUart();

	BuffaloGpio_MiconIntSetup();
	rv = request_irq(bit_micon_irq, micon_interrupts, 0, "MiCon", NULL);
	if (rv == 0)
		initialized = 1;
	return rv;
}

//--------------------------------------------------------------
void BuffaloMiconV2_exit(void)
{
	MV_U32 bit_micon_irq;

	if (!initialized)
		return;

	bit_micon_irq = IRQ_GPP_START + mvBoardGpioPinNumGet(BOARD_GPP_MC_IRQ, 0);

	free_irq(bit_micon_irq, NULL);
	TRACE(printk(">%s\n",__FUNCTION__));
	printk("MICON V2 removed.");
}

module_init(BuffaloMiconV2_init);
module_exit(BuffaloMiconV2_exit);
MODULE_LICENSE("GPL");

