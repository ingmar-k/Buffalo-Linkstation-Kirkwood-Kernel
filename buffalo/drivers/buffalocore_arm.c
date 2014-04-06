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

#include <linux/version.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/device.h>
#include <asm/uaccess.h>

#include "buffalo/buffalocore.h"
#include "buffalo/kernevntProc.h"
#include "mvTypes.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "ctrlEnv/mvCtrlEnvLib.h"

/* Globals */
// same as Buffalo Kernel Ver.
#define BUFCORE_VERSION "0.16"

/* Module parameters */
MODULE_AUTHOR("BUFFALO");
MODULE_DESCRIPTION("Buffalo Platform Linux Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(BUFCORE_VERSION);

/* Definitions */
//#define DEBUG

#define USE_PROC_BUFFALO

// ----------------------------------------------------

/* Definitions for DEBUG */
#ifdef DEBUG
 #define FUNCTRACE(x)  x

#else
 #define FUNCTRACE(x)

#endif

/* Function prototypes */

/* variables */
int buffalo_booting = 1;
u32 env_addr;
u32 env_size;
u32 env_offset;
static struct class *buffalo_cls;
#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
#include <asm/setup.h>

char umsg[UMSG_SIZE] = "\0";
int umsgi = 0;
#endif

#define DEBUG_PROC
#ifdef DEBUG_PROC
//----------------------------------------------------------------------
static int debug_proc_show(struct seq_file *m, void *v)
{
	unsigned base = 0xF1000000;
	volatile unsigned *gpio = (unsigned *)(base + 0x10100);
#if 0
	volatile struct tagReg {
		unsigned MainIntCause;
		unsigned MainIrqIntMask;
		unsigned MainFiqIntMask;
	} *intreg;

	
	intreg = (struct tagReg *)(base + 0x20200);
	
	seq_printf(m, "cause=%lx irqmask=%lx fiqmask=%lx\n",
		intreg->MainIntCause,
		intreg->MainIrqIntMask,
		intreg->MainFiqIntMask);

	seq_printf(m, "GPIO\n");
	seq_printf(m, "  OutReg=0x%04x\n", gpio[0]);
	seq_printf(m, "  DOutEn=0x%04x\n", gpio[1]);
	seq_printf(m, "  DBlink=0x%04x\n", gpio[2]);
	seq_printf(m, "  DatainActLow=0x%04x\n", gpio[3]);
	seq_printf(m, "  DIn=0x%04x\n", gpio[4]);
	seq_printf(m, "  Int=0x%04x\n", gpio[5]);
	seq_printf(m, "  Mask=0x%03x\n", gpio[6]);
	seq_printf(m, "  LMask=0x%04x\n", gpio[7]);
#endif

	seq_printf(m, "GPP_DATA_OUT_REG(0)   = 0x%08x\n", gpio[0]);
	seq_printf(m, "GPP_DATA_OUT_EN_REG(0)= 0x%08x\n", gpio[1]);
	seq_printf(m, "GPP_BLINK_EN_REG(0)   = 0x%08x\n", gpio[2]);
	seq_printf(m, "GPP_DATA_IN_POL_REG(0)= 0x%08x\n", gpio[3]);
	seq_printf(m, "GPP_DATA_IN_REG(0)    = 0x%08x\n", gpio[4]);
	seq_printf(m, "GPP_INT_CAUSE_REG(0)  = 0x%08x\n", gpio[5]);
	seq_printf(m, "GPP_INT_MASK_REG(0)   = 0x%08x\n", gpio[6]);
	seq_printf(m, "GPP_INT_LVL_REG(0)    = 0x%08x\n", gpio[7]);
	seq_printf(m, "MPP Control 0         = 0x%08x\n", *((unsigned *)(base+0x10000)));
	seq_printf(m, "MPP Control 1         = 0x%08x\n", *((unsigned *)(base+0x10004)));
	seq_printf(m, "MPP Control 2         = 0x%08x\n", *((unsigned *)(base+0x10050)));
	seq_printf(m, "Sample at Reset       = 0x%08x\n", *((unsigned *)(base+0x10010)));
	seq_printf(m, "Device Multiplex Control Register = 0x%08x\n", *((unsigned *)(base+0x10008)));
	
	return 0;
}

static int debug_proc_open(struct inode *inode, struct file *file)
{

	return single_open(file, debug_proc_show, NULL);
}

static const struct file_operations buffalo_debug_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= debug_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

#endif

/* Functions */
//----------------------------------------------------------------------
char * getdate(void)
{
	static char newstr[32];
	char sMonth[8];
	unsigned char i;
	int iDay,iYear,iMonth=0;
	const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug",
                            "Sep", "Oct", "Nov", "Dec"};

	
	sscanf(__DATE__,"%s %d %d",sMonth,&iDay,&iYear);
	
	for (i = 0; i < 12; i++){
		if (!strcmp(sMonth, months[i])){
			iMonth = i + 1;
			break;
		}
	}
	sprintf(newstr,"%d/%02d/%02d",iYear,iMonth,iDay);
	return newstr;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
extern char saved_command_line[];
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21) */

extern int buffaloBoardInfoInit(void);
extern u32 buffalo_product_id;
extern char buffalo_series_name[];
extern char buffalo_product_name[];
//----------------------------------------------------------------------
static int kernelfw_proc_show(struct seq_file *m, void *v)
{
	char *p, bootver[16];
	
	//printk("cmd:%s\n",saved_command_line);
	p = strstr(saved_command_line, "BOOTVER=");
	if (p) {
		strncpy(bootver, p, sizeof(bootver));
		p = strstr(bootver, " ");
		if (p)
			*p = 0;
	}

	seq_printf(m, "SERIES=%s\n", buffalo_series_name);
	seq_printf(m, "PRODUCTNAME=%s\n", buffalo_product_name);

	seq_printf(m, "VERSION=%s\n", BUFCORE_VERSION);
	seq_printf(m, "SUBVERSION=FLASH 0.00\n");

	seq_printf(m, "PRODUCTID=0x%08X\n", buffalo_product_id);
	
	seq_printf(m, "BUILDDATE=%s %s\n", getdate(), __TIME__);
	seq_printf(m, "%s\n", bootver);
	
	return 0;
}

static int kernelfw_proc_open(struct inode *inode, struct file *file)
{

	return single_open(file, kernelfw_proc_show, NULL);
}

static const struct file_operations buffalo_kernelfw_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= kernelfw_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

//----------------------------------------------------------------------
static int enetinfo_read_proc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	//int              i;
	int              len  = 0;
	char*            buf  = page;
	off_t begin = 0;
	int egiga_buffalo_proc_string(char *buf); // arch/arm/mach-mv88fxx81/egiga/mv_e_main.c

	len = egiga_buffalo_proc_string(buf);
	*start = page + (offset - begin);	/* Start of wanted data */
	len -= (offset - begin);	/* Start slop */
	if (len > length)
		len = length;	/* Ending slop */
	return (len);
}

static int enetinfo_write_proc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	extern void egiga_buffalo_change_configuration(MV_U8);

	//printk("buffer=%s\n", buffer);
	if(strncmp(buffer, "auto", 4) == 0){
		//printk("setting to auto...\n");
		egiga_buffalo_change_configuration((unsigned short)0);
	}else if(strncmp(buffer, "master", 6) == 0){
		//printk("setting to master...\n");
		egiga_buffalo_change_configuration((unsigned short)1);
	}else if(strncmp(buffer, "slave", 5) == 0){
		//printk("setting to slave...\n");
		egiga_buffalo_change_configuration((unsigned short)2);
	}else{
		//printk("no effect\n");
	}

	return count;
}
//----------------------------------------------------------------------
static int micon_proc_show(struct seq_file *m, void *v)
{

	seq_printf(m, "on\n");
	return 0;
}

static int micon_proc_open(struct inode *inode, struct file *file)
{

	return single_open(file, micon_proc_show, NULL);
}

static const struct file_operations buffalo_micon_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= micon_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

//----------------------------------------------------------------------
#if defined(CONFIG_BUFFALO_TERASTATION_TSHTGL)
static int lcd_proc_show(struct seq_file *m, void *v)
{

	seq_printf(m, "on\n");
	return 0;
}

static int lcd_proc_open(struct inode *inode, struct file *file)
{

	return single_open(file, lcd_proc_show, NULL);
}

static const struct file_operations buffalo_lcd_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= lcd_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

//----------------------------------------------------------------------
#if defined CONFIG_BUFFALO_SUPPORT_BOARD_INFO
static int board_info_proc_show(struct seq_file *m, void *v)
{
	char szBoardName[30];

	mvBoardNameGet(szBoardName);
	seq_printf(m, "BoardId=%02x\n", mvBoardIdGet());
	seq_printf(m, "BoardName=%s\n", szBoardName);
	//	seq_printf(m, "BoardStrap=%02x\n", BuffaloBoardGetStrapStatus());

	return 0;
}

static int board_info_proc_open(struct inode *inode, struct file *file)
{

	return single_open(file, board_info_proc_show, NULL);
}

static const struct file_operations buffalo_board_info_proc_fops = {
	.owner		= THIS_MODULE,
	.open		= board_info_proc_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};
#endif

//----------------------------------------------------------------------
static int booting_read_proc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	int len = sprintf(page, "%d\n", buffalo_booting);
	*eof = 1;
	return len;
}

//----------------------------------------------------------------------
static int booting_write_proc(struct file *filep, const char *buffer, unsigned long count, void *data)
{
	if (strncmp(buffer, "0\n", count) == 0)
	{
		buffalo_booting = 0;
	}
	else if (strncmp(buffer, "1\n", count) == 0)
	{
		buffalo_booting = 1;
	}

	return count;
}

static int
tj_read_proc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	volatile unsigned *tssr = (unsigned *)(0xf1010078);
	int tj, mod;

	tj = (322 - ((*tssr & 0x0007fc00) >> 10));
	mod = tj * 1000000 / 13625 % 100;
	if (mod < 0) mod = -mod;
	tj = tj * 10000 / 13625;

	len = sprintf(page, "%d.%02d\n", tj, mod);

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}

#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
static int umsg_read_proc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	int len;

	if (umsgi >= strlen(umsg)) {
		if (offset == 0)
			umsgi = 0;
		*eof = 1;
	}

	len = snprintf(page, length, "%s", &umsg[umsgi]);
	*start = page;
	if (len > length)
		len = length - 1;

	umsgi += len;

	return len;
}
#endif

struct class *BuffaloClassGet(void)
{
	struct class *tmp_cls;

	if (buffalo_cls)
		return buffalo_cls;

	tmp_cls = class_create(THIS_MODULE, "buffalo");

	if (IS_ERR(tmp_cls)) {
		printk(">%s: class_create failed.", __FUNCTION__);
		return NULL;
	}

	buffalo_cls = tmp_cls;
	return buffalo_cls;
}

static ssize_t show_env_addr(struct device *dev, struct device_attribute *attr, char *buf)
{
  return sprintf(buf, "0x%08x\n", env_addr);
}

static ssize_t show_env_size(struct device *dev, struct device_attribute *attr, char *buf)
{
  return sprintf(buf, "0x%08x\n", env_size);
}

static ssize_t show_env_offset(struct device *dev, struct device_attribute *attr, char *buf)
{
  return sprintf(buf, "0x%08x\n", env_offset);
}

DEVICE_ATTR(env_addr, S_IRUSR, show_env_addr, NULL);
DEVICE_ATTR(env_size, S_IRUSR, show_env_size, NULL);
DEVICE_ATTR(env_offset, S_IRUSR, show_env_offset, NULL);

struct proc_dir_entry *
get_proc_buffalo(void)
{
	static struct proc_dir_entry *buffalo = NULL;

	if (buffalo == NULL)
		buffalo = proc_mkdir("buffalo", NULL);

	return buffalo;
}

//----------------------------------------------------------------------
int __init buffaloDriver_init (void)
{
	struct proc_dir_entry *buffalo, *enet, *booting, *tj;
	struct class *cls;
	FUNCTRACE(printk(">%s\n",__FUNCTION__));

	buffalo = get_proc_buffalo();
	proc_create("firmware", 0, buffalo, &buffalo_kernelfw_proc_fops);
#ifdef DEBUG_PROC
	proc_create("debug", 0, buffalo, &buffalo_debug_proc_fops);
#endif
	enet = create_proc_entry ("enet", 0, buffalo);
	enet->read_proc = &enetinfo_read_proc;
	enet->write_proc= &enetinfo_write_proc;

#ifdef CONFIG_BUFFALO_USE_MICON
	proc_create("micon", 0, buffalo, &buffalo_micon_proc_fops);
#endif
#if defined CONFIG_BUFFALO_SUPPORT_BOARD_INFO
	proc_create("board_info", 0, buffalo, &buffalo_board_info_proc_fops);
#endif

	booting = create_proc_entry("booting", 0, buffalo);
	booting->read_proc = &booting_read_proc;
	booting->write_proc = &booting_write_proc;

	BuffaloKernevnt_init();
#if defined(CONFIG_BUFFALO_TERASTATION_TSHTGL)
	proc_create("lcd", 0, buffalo, &buffalo_lcd_proc_fops);
#endif

	cls = BuffaloClassGet();
	if (cls) {
		int rv;

		struct device *dev = device_create(cls, NULL, 0, NULL, "u-boot");
		rv = device_create_file(dev, &dev_attr_env_addr);
		rv = device_create_file(dev, &dev_attr_env_size);
		rv = device_create_file(dev, &dev_attr_env_offset);
	}

	if (mvCtrlModelGet() == MV_6282_DEV_ID) {
		tj = create_proc_entry("tj", 0, buffalo);
		tj->read_proc = tj_read_proc;
	}

#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
	{
		struct proc_dir_entry *umsg;

		umsg = create_proc_entry("umsg", 0, buffalo);
		umsg->read_proc = &umsg_read_proc;
	}
#endif

	return 0;
}


//----------------------------------------------------------------------
void buffaloDriver_exit(void)
{
	struct proc_dir_entry *buffalo;

	FUNCTRACE(printk(">%s\n",__FUNCTION__));

	BuffaloKernevnt_exit();
	buffalo = get_proc_buffalo();
#if defined CONFIG_BUFFALO_SUPPORT_BOARD_INFO
	remove_proc_entry("board_info", buffalo);
#endif
	remove_proc_entry("booting", buffalo);
#if defined CONFIG_BUFFALO_USE_MICON
	remove_proc_entry("micon", buffalo);
#endif
	remove_proc_entry("enet", buffalo);
#ifdef DEBUG_PROC
	remove_proc_entry("debug", buffalo);
#endif
	remove_proc_entry("firmware", buffalo);
	remove_proc_entry ("buffalo", 0);
}

module_init(buffaloDriver_init);
module_exit(buffaloDriver_exit);
