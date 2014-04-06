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
#include <linux/init.h>
#include <linux/device.h>
#include <asm/uaccess.h>

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

//#include "kernevntProc.h"
#include "buffalo/kernevnt.h"

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
int bfMagicKey = MagicKeyHwPoff;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
extern char saved_command_line[];
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21) */

#if defined(NEW_STYLE)
//----------------------------------------------------------------------
static int kernelfw_proc_show(struct seq_file *m, void *v)
{
#if 0
	char *p, *pe, bootver[16], buffalo_series_name[16], buffalo_product_name[16], buffalo_product_id[16];
	memset(bootver, 0, sizeof(bootver));
	memset(buffalo_series_name, 0, sizeof(buffalo_series_name));
	memset(buffalo_product_name, 0, sizeof(buffalo_product_name));
	memset(buffalo_product_id, 0, sizeof(buffalo_product_id));

#if 0
	printk("cmd:%s\n",saved_command_line);
	p = strstr(saved_command_line,"BOOTVER=");
	if (p)
	{
		pe = strstr(p, " ");

		if(pe && (pe - p) < sizeof(bootver))
			strncpy(bootver, p, pe - p);
		else
			strncpy(bootver,p,sizeof(bootver));
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
		sprintf(buffalo_series_name, "Unkown");
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
		sprintf(buffalo_product_name, "Unkown");
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
		sprintf(buffalo_product_id, "0x00000000");
	printk("buffalo_product_id=%s\n", buffalo_product_id);
#else
	sprintf(buffalo_series_name, "TeraStation");
	sprintf(buffalo_product_name, "TS-VHL");
	sprintf(buffalo_product_id, "0x00000000");
	sprintf(bootver, "Unknown");
#endif

	seq_printf(m, "SERIES=%s\n", buffalo_series_name);
	seq_printf(m, "PRODUCTNAME=%s\n", buffalo_product_name);

	seq_printf(m, "VERSION=%s\n",BUFCORE_VERSION);
	seq_printf(m, "SUBVERSION=FLASH 0.00\n");

#if 0
	seq_printf(m, "PRODUCTID=0x%08X\n", buffalo_product_id);
#else
	seq_printf(m, "PRODUCTID=%s\n", buffalo_product_id);
#endif

	seq_printf(m, "BUILDDATE=<<<!!!BUFFALO_BUILD_DATE!!!>>>\n");
	seq_printf(m, "%s\n",bootver);
#else
	seq_printf(m, "SERIES=TeraStation\n");
	seq_printf(m, "PRODUCTNAME=TS-VHL(KOHTOKU)\n");

	seq_printf(m, "VERSION=%s\n",BUFCORE_VERSION);
	seq_printf(m, "SUBVERSION=FLASH 0.00\n");

	seq_printf(m, "PRODUCTID=0x00002012\n");

	seq_printf(m, "BUILDDATE=<<<!!!BUFFALO_BUILD_DATE!!!>>>\n");
	seq_printf(m, "BOOTVER=0.00\n");
#endif
	return 0;
}

static const struct file_operations buffalo_kernelfw_proc_fops = {
	.owner          = THIS_MODULE,
	.open           = kernelfw_proc_show,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};
#else
static char bootver[32];
static char buffalo_series_name[32];
static char buffalo_product_name[32];
static char buffalo_product_id[32];

static int kernelfw_read_proc(char *page, char **start, off_t offset, int length)
{
        int              len  = 0;
        char*            buf  = page;
        off_t begin = 0;
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
			sprintf(buffalo_product_name, "TS-QVHL/R5(SAIMEI)");
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
			sprintf(buffalo_product_id, "0x00002013");
	}
	

        len += sprintf(buf+len,"SERIES=%s\n", buffalo_series_name);
        len += sprintf(buf+len,"PRODUCTNAME=%s\n", buffalo_product_name);

        len += sprintf(buf+len,"VERSION=%s\n",BUFCORE_VERSION);
        len += sprintf(buf+len,"SUBVERSION=FLASH 0.00\n");

        len += sprintf(buf+len,"PRODUCTID=%s\n", buffalo_product_id);

        len += sprintf(buf+len,"BUILDDATE=2010/07/14 00:00:00\n");
        len += sprintf(buf+len,"%s\n",bootver);

        *start = page + (offset - begin);       /* Start of wanted data */
        len -= (offset - begin);        /* Start slop */
        if (len > length)
                len = length;   /* Ending slop */
        return (len);
}

#endif

#if defined(CONFIG_BUFFALO_USE_MICON) || defined(CONFIG_BUFFALO_MATSU_PLATFORM)
#if defined(NEW_STYLE)
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
	.owner          = THIS_MODULE,
	.open           = micon_proc_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
};

static int miconint_proc_show(struct seq_file *m, void *v)
{
	return 0;
}

static int miconint_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, miconint_proc_show, NULL);
}

static const struct file_operations  buffalo_miconint_proc_fops = {
	.owner          = THIS_MODULE,
	.open           = miconint_proc_open,
	.read           = seq_read,
	.llseek         = seq_lseek,
	.release        = single_release,
}
#else
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

#endif // OF NEW_STYLE
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
        struct proc_dir_entry *generic, *buffalo, *booting;
        FUNCTRACE(printk(">%s\n",__FUNCTION__));

	buffalo = get_proc_buffalo();
#if defined(NEW_STYLE)
	proc_create("firmware", 0, buffalo, &buffalo_kernelfw_proc_fops);
#else
	generic = create_proc_entry ("firmware", 0, buffalo);
	generic->read_proc=&kernelfw_read_proc;
#endif

#if defined(CONFIG_BUFFALO_USE_MICON) || defined(CONFIG_BUFFALO_MATSU_PLATFORM)
#if defined(NEW_STYLE)
        proc_create("micon", 0, buffalo, &buffalo_micon_proc_fops);
	proc_create("miconint_en", 0, buffalo, &buffalo_miconint_proc_fops);
#else
	generic = create_proc_entry("micon", 0, buffalo);
	generic->read_proc = &micon_read_proc;
	generic = create_proc_entry("miconint_en", 0, buffalo);
	generic->read_proc = &MiconIntActivate_read_proc;
#endif
#endif

	generic = create_proc_entry("cpu_status", 0, buffalo);
	generic->read_proc = &BuffaloCpuStatusReadProc;
	generic->write_proc= &BuffaloCpuStatusWriteProc;

	booting = create_proc_entry("booting", 0, buffalo);
	booting->read_proc = &booting_read_proc;
	booting->write_proc = &booting_write_proc;

//	BuffaloKernevnt_init();

	return 0;
}

//----------------------------------------------------------------------
void buffaloDriver_exit(void)
{
	FUNCTRACE(printk(">%s\n",__FUNCTION__));

	//BuffaloKernevnt_exit();
	remove_proc_entry("buffalo/booting", 0);
	remove_proc_entry("buffalo/cpu_status", 0);
#if defined(CONFIG_BUFFALO_USE_MICON) || defined(CONFIG_BUFFALO_MATSU_PLATFORM)
	remove_proc_entry("buffalo/miconint_en", 0);
	remove_proc_entry("buffalo/micon", 0);
#endif
	remove_proc_entry("buffalo/firmware", 0);
	remove_proc_entry ("buffalo", 0);
}

module_init(buffaloDriver_init);
module_exit(buffaloDriver_exit);
