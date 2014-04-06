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

#ifdef CONFIG_X86
#include <linux/fs.h>
#include <linux/seq_file.h>

//#include "kernevntProc.h"
#include "buffalo/kernevnt.h"
#else // CONFIG_X86
#include "buffalocore.h"
#include "kernevntProc.h"
#include "mvTypes.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#endif // CONFIG_X86
#include "buffalo_proc_entry.h"

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

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
extern char saved_command_line[];
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21) */

// begin kernelfw
#ifdef CONFIG_X86
static char bootver[32];
static char buffalo_series_name[32];
static char buffalo_product_name[32];
static char buffalo_product_id[32];
#else // CONFIG_X86
extern u32 buffalo_product_id;
extern char buffalo_series_name[];
extern char buffalo_product_name[];

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
#endif // CONFIG_X86

static int kernelfw_read_proc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
        int              len  = 0;
        char*            buf  = page;
        off_t begin = 0;
#ifdef CONFIG_X86
	char *p, *pe;

	if (!(buffalo_series_name[0] >= 'a' && buffalo_series_name[0] <= 'z') &&
		!(buffalo_series_name[0] >= 'A' && buffalo_series_name[0] <= 'Z'))
	{
		memset(bootver, 0, sizeof(bootver));
		memset(buffalo_series_name, 0, sizeof(buffalo_series_name));
		memset(buffalo_product_name, 0, sizeof(buffalo_product_name));
		memset(buffalo_product_id, 0, sizeof(buffalo_product_id));

		p = strstr(saved_command_line,"BOOTVER=");
		if (p)
		{
			pe = strstr(p, " ");

			if(pe && (pe - p) < sizeof(bootver))
				strncpy(bootver, p, pe - p);
			else
				strncpy(bootver, p, sizeof(bootver));
		}
		else
			sprintf(bootver, "BOOTVER=Unkown");
		printk("bootver=%s\n", bootver);

		p = strstr(saved_command_line, "SERIES=");
		if (p)
		{
			p += strlen("SERIES=");
			pe = strstr(p, " ");

			if (pe && (pe - p) < sizeof(buffalo_series_name))
				strncpy(buffalo_series_name, p, pe - p);
			else
				strncpy(buffalo_series_name, p, sizeof(buffalo_series_name));
		}
		else
			sprintf(buffalo_series_name, "TeraStation");
		printk("buffalo_series_name=%s\n", buffalo_series_name);

		p = strstr(saved_command_line, "PRODUCTNAME=");
		if (p)
		{
			p += strlen("PRODUCTNAME=");
			pe = strstr(p, " ");

			if (pe && (pe - p) < sizeof(buffalo_product_name))
				strncpy(buffalo_product_name, p, pe - p);
			else
				strncpy(buffalo_product_name, p, sizeof(buffalo_product_name));
		}
		else
#if defined(CONFIG_BUFFALO_MATSU_PLATFORM)
			sprintf(buffalo_product_name, "TS-QVHL/R5(SAIMEI)");
#elif defined(CONFIG_BUFFALO_KIRI_PLATFORM)
			sprintf(buffalo_product_name, "TS-2URVHL/R5(SAIMEI)");
#else
	#error
#endif
		printk("buffalo_product_name=%s\n", buffalo_product_name);

		p = strstr(saved_command_line, "PRODUCTID=");
		if (p)
		{
			p += strlen("PRODUCTID=");
			pe = strstr(p, " ");

			if (pe && (pe - p) < sizeof(buffalo_product_id))
				strncpy(buffalo_product_id, p, pe - p);
			else
				strncpy(buffalo_product_id, p, sizeof(buffalo_product_id));
		}
		else
#if defined(CONFIG_BUFFALO_MATSU_PLATFORM)
			sprintf(buffalo_product_id, "0x00002013");
#elif defined(CONFIG_BUFFALO_KIRI_PLATFORM)
			sprintf(buffalo_product_id, "0x00002017");
#else
	#error
#endif
	}
#else
	char *p, bootver[16];
	
	//printk("cmd:%s\n",saved_command_line);
	p = strstr(saved_command_line, "BOOTVER=");
	if (p) {
		strncpy(bootver, p, sizeof(bootver));
		p = strstr(bootver, " ");
		if (p)
			*p = 0;
	}
#endif // CONFIG_X86

        len += sprintf(buf+len,"SERIES=%s\n", buffalo_series_name);
        len += sprintf(buf+len,"PRODUCTNAME=%s\n", buffalo_product_name);

        len += sprintf(buf+len,"VERSION=%s\n",BUFCORE_VERSION);
        len += sprintf(buf+len,"SUBVERSION=FLASH 0.00\n");

#ifdef CONFIG_X86
        len += sprintf(buf+len,"PRODUCTID=%s\n", buffalo_product_id);
#else // CONFIG_X86
        len += sprintf(buf+len,"PRODUCTID=0x%08X\n", buffalo_product_id);
#endif // CONFIG_X86

	//len += sprintf(buf+len,"BUILDDATE=2012/06/19 12:20:32\n");
	len += sprintf(buf+len,"BUILDDATE=%s %s\n", getdate(), __TIME__);
        len += sprintf(buf+len,"%s\n",bootver);

        *start = page + (offset - begin);       /* Start of wanted data */
        len -= (offset - begin);        /* Start slop */
        if (len > length)
                len = length;   /* Ending slop */
        return (len);
}

static struct proc_entry_data firmware_entry =
{
	.path = "firmware",
	.mode = 0,
	.readproc = &kernelfw_read_proc,
	.writeproc = NULL,
	.data = NULL,
};
// end kernelfw

// begin micon
#if /*!defined(CONFIG_X86) ||*/ defined(CONFIG_BUFFALO_USE_MICON) || defined(CONFIG_BUFFALO_MATSU_PLATFORM)
static int micon_read_proc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	int len  = 0;
	if (offset > 0)
	{
		*eof = 1;
		return 0;
	}

	len = sprintf(page,"on\n");
	*eof = 1;
	return len;
}

static struct proc_entry_data micon_entry =
{
	.path = "micon",
	.mode = 0,
	.readproc = &micon_read_proc,
	.writeproc = NULL,
	.data = NULL,
};
#endif
// end micon

// begin booting
int buffalo_booting = 1;

static int booting_read_proc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	int len = sprintf(page, "%d\n", buffalo_booting);
	*eof = 1;
	return len;
}

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

static struct proc_entry_data booting_entry =
{
	.path = "booting",
	.mode = 0,
	.readproc = &booting_read_proc,
	.writeproc = &booting_write_proc,
	.data = NULL,
};
// end booting

#ifdef CONFIG_X86
// begin miconint_en
#if defined(CONFIG_BUFFALO_USE_MICON) || defined(CONFIG_BUFFALO_MATSU_PLATFORM)
static int MiconIntActivate_read_proc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	int len;
	if (offset > 0)
	{
		*eof = 1;
		return 0;
	}

	len = sprintf(page, "MiconAct\n");

	*eof = 1;
	return len;
}

static struct proc_entry_data miconint_en_entry =
{
	.path = "miconint_en",
	.mode = 0,
	.readproc = &MiconIntActivate_read_proc,
	.writeproc = NULL,
	.data = NULL,
};
#endif
// end miconint_en

// begin cpu_status
int bfMagicKey = MagicKeyHwPoff;

static int
bfGetMagicKey(void)
{
	return bfMagicKey;
}

static void
bfSetMagicKey(int key)
{ 
	bfMagicKey = key;
}

static int
BuffaloCpuStatusReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	unsigned int CpuStatus = bfGetMagicKey();

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
	case MagicKeyWOLReadyState:
		len = sprintf(page, "WOLReady\n");
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
		bfSetMagicKey(MagicKeyReboot);
	}else if(strcmp(status, "reboot_uboot_passed") == 0){
		bfSetMagicKey(MagicKeyRebootUbootPassed);
	}else if(strcmp(status, "normal_state") == 0){
		bfSetMagicKey(MagicKeyNormalState);
	}else if(strcmp(status, "hwpoff") == 0){
		bfSetMagicKey(MagicKeyHwPoff);
	}else if(strcmp(status, "swpoff") == 0){
		bfSetMagicKey(MagicKeySwPoff);
	}else if(strcmp(status, "swpoff_uboot_passed") == 0){
		bfSetMagicKey(MagicKeySWPoffUbootPassed);
	}else if(strcmp(status, "fwup") == 0){
		bfSetMagicKey(MagicKeyFWUpdating);
	}else if(strcmp(status, "ups_shutdown") == 0){
		bfSetMagicKey(MagicKeyUpsShutdown);
	}else if(strcmp(status, "WOLReady") == 0){
		bfSetMagicKey(MagicKeyWOLReadyState);
	}else{

	}

	return count;
}

static struct proc_entry_data cpu_status_entry =
{
	.path = "cpu_status",
	.mode = 0,
	.readproc = &BuffaloCpuStatusReadProc,
	.writeproc = &BuffaloCpuStatusWriteProc,
	.data = NULL,
};
// end cpu_status

#else // CONFIG_X86

// begin debug
#define DEBUG_PROC
#ifdef DEBUG_PROC
static int debug_read_proc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
        int              len  = 0;
        char*            buf  = page;
        off_t begin = 0;

	unsigned base = 0xF1000000;
	volatile unsigned *gpio = (unsigned *)(base + 0x10100);
#if 0
	volatile struct tagReg {
		unsigned MainIntCause;
		unsigned MainIrqIntMask;
		unsigned MainFiqIntMask;
	} *intreg;

	
	intreg = (struct tagReg *)(base + 0x20200);
	
	len += sprintf(buf+len, "cause=%lx irqmask=%lx fiqmask=%lx\n",
		       intreg->MainIntCause,
		       intreg->MainIrqIntMask,
		       intreg->MainFiqIntMask);

	len += sprintf(buf+len, "GPIO\n");
	len += sprintf(buf+len, "  OutReg=0x%04x\n", gpio[0]);
	len += sprintf(buf+len, "  DOutEn=0x%04x\n", gpio[1]);
	len += sprintf(buf+len, "  DBlink=0x%04x\n", gpio[2]);
	len += sprintf(buf+len, "  DatainActLow=0x%04x\n", gpio[3]);
	len += sprintf(buf+len, "  DIn=0x%04x\n", gpio[4]);
	len += sprintf(buf+len, "  Int=0x%04x\n", gpio[5]);
	len += sprintf(buf+len, "  Mask=0x%03x\n", gpio[6]);
	len += sprintf(buf+len, "  LMask=0x%04x\n", gpio[7]);
#endif

	len += sprintf(buf+len, "GPP_DATA_OUT_REG(0)   = 0x%08x\n", gpio[0]);
	len += sprintf(buf+len, "GPP_DATA_OUT_EN_REG(0)= 0x%08x\n", gpio[1]);
	len += sprintf(buf+len, "GPP_BLINK_EN_REG(0)   = 0x%08x\n", gpio[2]);
	len += sprintf(buf+len, "GPP_DATA_IN_POL_REG(0)= 0x%08x\n", gpio[3]);
	len += sprintf(buf+len, "GPP_DATA_IN_REG(0)    = 0x%08x\n", gpio[4]);
	len += sprintf(buf+len, "GPP_INT_CAUSE_REG(0)  = 0x%08x\n", gpio[5]);
	len += sprintf(buf+len, "GPP_INT_MASK_REG(0)   = 0x%08x\n", gpio[6]);
	len += sprintf(buf+len, "GPP_INT_LVL_REG(0)    = 0x%08x\n", gpio[7]);
	len += sprintf(buf+len, "MPP Control 0         = 0x%08x\n", *((unsigned *)(base+0x10000)));
	len += sprintf(buf+len, "MPP Control 1         = 0x%08x\n", *((unsigned *)(base+0x10004)));
	len += sprintf(buf+len, "MPP Control 2         = 0x%08x\n", *((unsigned *)(base+0x10050)));
	len += sprintf(buf+len, "Sample at Reset       = 0x%08x\n", *((unsigned *)(base+0x10010)));
	len += sprintf(buf+len, "Device Multiplex Control Register = 0x%08x\n", *((unsigned *)(base+0x10008)));
	
        *start = page + (offset - begin);       /* Start of wanted data */
        len -= (offset - begin);        /* Start slop */
        if (len > length)
                len = length;   /* Ending slop */
        return (len);
}

static struct proc_entry_data debug_entry =
{
	.path = "debug",
	.mode = 0,
	.readproc = &debug_read_proc,
	.writeproc = NULL,
	.data = NULL,
};
#endif
// end debug

// begin enet
#if !defined(CONFIG_ARCH_ARMADA370) && !defined(CONFIG_ARCH_ARMADA_XP)
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

static struct proc_entry_data enet_entry =
{
	.path = "enet",
	.mode = 0,
	.readproc = &enetinfo_read_proc,
	.writeproc = &enetinfo_write_proc,
	.data = NULL,
};
#endif // CONFIG_ARCH_ARMADA370
// end enet

// begin lcd
#if defined(CONFIG_BUFFALO_TERASTATION_TSHTGL)
static int lcd_read_proc(char *page, char **start, off_t offset, int length, int *eof, void *data)
{
	int len  = 0;
	if (offset > 0)
	{
		*eof = 1;
		return 0;
	}

	len = sprintf(page,"on\n");
	*eof = 1;
	return len;
}

static struct proc_entry_data lcd_entry =
{
	.path = "lcd",
	.mode = 0,
	.readproc = &lcd_read_proc,
	.writeproc = NULL,
	.data = NULL,
};
#endif
// end lcd

// begin board_info
#if defined CONFIG_BUFFALO_SUPPORT_BOARD_INFO
static int board_info_read_proc(char *page, char **start, off_t offset, int length)
{
        int              len  = 0;
        char*            buf  = page;
        off_t begin = 0;

	char szBoardName[30];

	mvBoardNameGet(szBoardName);
        len += sprintf(buf+len,"BoardId=%02x\n", mvBoardIdGet());
        len += sprintf(buf+len,"BoardName=%s\n", szBoardName);

        *start = page + (offset - begin);       /* Start of wanted data */
        len -= (offset - begin);        /* Start slop */
        if (len > length)
                len = length;   /* Ending slop */
        return (len);
}

static struct proc_entry_data board_info_entry =
{
	.path = "board_info",
	.mode = 0,
	.readproc = &board_info_read_proc,
	.writeproc = NULL,
	.data = NULL,
};
#endif
// end board_info

// begin tj
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

static struct proc_entry_data tj_entry =
{
	.path = "tj",
	.mode = 0,
	.readproc = &tj_read_proc,
	.writeproc = NULL,
	.data = NULL,
};
// end tj

// begin umsg
#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
#include <asm/setup.h>

char umsg[UMSG_SIZE] = "\0";
int umsgi = 0;

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

static struct proc_entry_data umsg_entry =
{
	.path = "umsg",
	.mode = 0,
	.readproc = &umsg_read_proc,
	.writeproc = NULL,
	.data = NULL,
};
#endif
// end umsg

u32 env_addr;
u32 env_size;
u32 env_offset;
static struct class *buffalo_cls;

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
#endif // CONFIG_X86

static struct proc_entry_data *ped_list[] = {
	&firmware_entry,
#if defined(CONFIG_BUFFALO_USE_MICON) || defined(CONFIG_BUFFALO_MATSU_PLATFORM)
	&micon_entry,
#endif
	&booting_entry,
#ifdef CONFIG_X86
#if defined(CONFIG_BUFFALO_USE_MICON) || defined(CONFIG_BUFFALO_MATSU_PLATFORM)
	&miconint_en_entry,
#endif
	&cpu_status_entry,
#else // CONFIG_X86
#ifdef DEBUG_PROC
	&debug_entry,
#endif
#if !defined(CONFIG_ARCH_ARMADA370) && !defined(CONFIG_ARCH_ARMADA_XP)
	&enet_entry,
#endif // CONFIG_ARCH_ARMADA370
#if defined(CONFIG_BUFFALO_TERASTATION_TSHTGL)
	&lcd_entry,
#endif
#if defined CONFIG_BUFFALO_SUPPORT_BOARD_INFO
	&board_info_entry,
#endif
#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
	&umsg_entry,
#endif
#endif // CONFIG_X86
	NULL,
};

//----------------------------------------------------------------------
int __init buffaloDriver_init (void)
{
	int i;
	struct class *cls;

        FUNCTRACE(printk(">%s\n",__FUNCTION__));

	BuffaloKernevnt_init();

	for (i = 0; ped_list[i]; i++) {
		make_proc_entry(ped_list[i]);
	}

	cls = BuffaloClassGet();
	if (cls) {
		int rv;

		struct device *dev = device_create(cls, NULL, 0, NULL, "u-boot");
		rv = device_create_file(dev, &dev_attr_env_addr);
		rv = device_create_file(dev, &dev_attr_env_size);
		rv = device_create_file(dev, &dev_attr_env_offset);
	}

	if (mvCtrlModelGet() == MV_6282_DEV_ID) {
		make_proc_entry(&tj_entry);
	}

	return 0;
}

//----------------------------------------------------------------------
void buffaloDriver_exit(void)
{
	int i;
	struct proc_dir_entry *buffalo;

	FUNCTRACE(printk(">%s\n",__FUNCTION__));

	BuffaloKernevnt_exit();
	buffalo = get_proc_buffalo();
	for (i = 0; ped_list[i]; i++) {
		remove_proc_entry(ped_list[i]->path, buffalo);
	}
	
	if (mvCtrlModelGet() == MV_6282_DEV_ID) {
		remove_proc_entry(tj_entry.path, buffalo);
	}

	remove_proc_entry ("buffalo", 0);
}

module_init(buffaloDriver_init);
module_exit(buffaloDriver_exit);
