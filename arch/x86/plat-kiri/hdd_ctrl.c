/*
 * HDD control driver - 0.1
 * 1. Detect if HDD is presented or not.
 * 2. Get the power status of HDD.
 * 3. Set the power status of HDD.
 *
 * Author: Simon Chang, USI.
 *
 * HDD control driver - 0.2
 * Extented to support 8 HDD
 *
 * Author: Wayne Lai, USI.
 *
 * History:
 * 0.1 Initial version for 4 HDDs.
 * 0.2 Extented to support 8 HDDs.
 */

#include <linux/pci.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/proc_fs.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include "hdd_ctrl.h"
#include "buffalo/kernevnt.h"

#define HDD_CTRL_MODULE_VERSION "0.2"
#define HDD_CTRL_MODULE_NAME "hdd control module"

static unsigned int ich9r_gpio_base_addr;

struct matsu_hdd_info_st {
	uint32_t HDD_GPIO_PRESENT_REG;
	uint32_t HDD_PRESENT_BIT;
	uint32_t HDD_GPIO_POWER_REG;
	uint32_t HDD_POWER_BIT;
	uint32_t HDD_POWER_MASK;
};

static struct matsu_hdd_info_st matsu_hdd_info[] = {
	{GP_LVL,	HDD0_PRESENT_BIT,	GP_LVL,		HDD0_POWER_BIT,	HDD0_POWER_MASK},
	{GP_LVL,	HDD1_PRESENT_BIT,	GP_LVL,		HDD1_POWER_BIT,	HDD1_POWER_MASK},
	{GP_LVL,	HDD2_PRESENT_BIT,	GP_LVL,		HDD2_POWER_BIT,	HDD2_POWER_MASK},
	{GP_LVL,	HDD3_PRESENT_BIT,	GP_LVL,		HDD3_POWER_BIT,	HDD3_POWER_MASK},
	{GP_LVL,	HDD4_PRESENT_BIT,	GP_LVL,		HDD4_POWER_BIT,	HDD4_POWER_MASK},
	{GP_LVL,	HDD5_PRESENT_BIT,	GP_LVL2,	HDD5_POWER_BIT,	HDD5_POWER_MASK},
	{GP_LVL,	HDD6_PRESENT_BIT,	GP_LVL2,	HDD6_POWER_BIT,	HDD6_POWER_MASK},
	{GP_LVL,	HDD7_PRESENT_BIT,	GP_LVL2,	HDD7_POWER_BIT,	HDD7_POWER_MASK},
	{GP_LVL,	HDD8_PRESENT_BIT,	GP_LVL,		HDD8_POWER_BIT, HDD8_POWER_MASK},
	{GP_LVL,	HDD9_PRESENT_BIT,	GP_LVL,		HDD9_POWER_BIT, HDD9_POWER_MASK},
	{GP_LVL,	HDD10_PRESENT_BIT,	GP_LVL,		HDD10_POWER_BIT, HDD10_POWER_MASK},
	{GP_LVL,	HDD11_PRESENT_BIT,	GP_LVL,		HDD11_POWER_BIT, HDD11_POWER_MASK},
};

static struct proc_dir_entry *hdd_power_dir;
static struct proc_dir_entry *hdd_present_dir;

struct timer_list sata_hotplug_polling_timer;
struct sata_hotplug_data_st sata_hotplug_data[MAX_SUPPORTED_DISKS];

/*******************************************************************************
* function: read_hdd_present_status
* argument:
* retvalue:
*******************************************************************************/
static int read_hdd_present_status(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;
	if (off > 0) {
		*eof = 1;
		return 0;
	}

	len = sprintf(page, "plugged\n");

	*eof = 1;
	return len;
}

/*******************************************************************************
* function: read_hdd_power_status
* argument:
* retvalue:
*******************************************************************************/
static int read_hdd_power_status(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;

	if (off > 0) {
		*eof = 1;
		return 0;
	}

	len = sprintf(page, "on\n");

	*eof = 1;
	return len;

}

/*******************************************************************************
* function: set_hdd_power
* argument:
* retvalue:
*******************************************************************************/
static int set_hdd_power(struct file *file, const char *buffer, unsigned long count, void *data)
{
	return count;
}

/*******************************************************************************
* function: init_proc_files
* argument:
* retvalue:
* description: create procfs for hdd control:
/ --- proc --- buffalo --- gpio -+- hotplug -+- sata0
                                 |           +- sata1
                                 |           +- sata2
                                 |           +- sata3
                                 |           +- sata4
                                 |           +- sata5
                                 |           +- sata6
                                 |           +- sata7
                                 |
                                 +- power_control -+- hdd0
                                                   +- hdd1
                                                   +- hdd2
                                                   +- hdd3
                                                   +- hdd4
                                                   +- hdd5
                                                   +- hdd6
                                                   +- hdd7
*******************************************************************************/
static int init_proc_files(void)
{
	int retval = 0;
	int i = 0;
	char buf[32];

	/* declare for file and directory of procfs */
	static struct proc_dir_entry *hdd_ctrl_dir;

	static struct proc_dir_entry *hdd_present_status_file[MAX_SUPPORTED_DISKS];
	static struct proc_dir_entry *hdd_power_status_file[MAX_SUPPORTED_DISKS];

	// Create /proc/buffalo/gpio directory
	hdd_ctrl_dir = proc_mkdir(MODULE_DIR, NULL);
	if(hdd_ctrl_dir == NULL) {
		printk(KERN_ALERT "buffalo/gpio directory create fail.....\n");
		goto MODULE_DIR_FAIL;
	}
	printk(KERN_ALERT "buffalo/gpio directory create successful.....\n");

	// Create /proc/buffalo/gpio/hotplug directory
	hdd_present_dir = proc_mkdir(HDD_PRESENT_DIR, NULL);
	if(hdd_present_dir == NULL) {
		printk(KERN_ALERT "hotplug directory create fail.....\n");
		goto PRESENT_DIR_FAIL;
	}
	printk(KERN_ALERT "hotplug directory create successful.....\n");

	for(i = 0; i < MAX_SUPPORTED_DISKS; i++)
	{
		sprintf(buf, "sata%d", i);
		hdd_present_status_file[i] = create_proc_read_entry(buf, 0, hdd_present_dir, read_hdd_present_status, NULL);
		if(hdd_present_status_file[i] == NULL)
		{
			printk(KERN_ALERT "HDD%d_PRESENT_STATUS_FILE create fail.....\n", i);
			goto HDD_PRESENT_STATUS_FILE_FAIL;
		}
		printk(KERN_ALERT "HDD%d_PRESENT_STATUS_FILE create successful.....\n", i);
		hdd_present_status_file[i]->data = (void *)&matsu_hdd_info[i];
	}

	// Create /proc/buffalo/gpio/power_control directory
	hdd_power_dir = proc_mkdir(HDD_POWER_DIR, NULL);
	if(hdd_power_dir == NULL) {
		printk(KERN_ALERT "power_control directory create fail.....\n");
		goto HDD_POWER_DIR_FAIL;
	}
	printk(KERN_ALERT "power_control directory create successful.....\n");

	for(i = 0; i < MAX_SUPPORTED_DISKS; i++)
	{
		sprintf(buf, "hdd%d", i);
		hdd_power_status_file[i] = create_proc_entry(buf, 0644, hdd_power_dir);
		if(hdd_power_status_file[i] == NULL)
		{
			printk(KERN_ALERT "HDD%d_POWER_STATUS_FILE create fail.....\n", i);
			goto HDD_POWER_STATUS_FILE_FAIL;
		}
		printk(KERN_ALERT "HDD%d_POWER_STATUS_FILE create successful.....\n", i);
		hdd_power_status_file[i]->read_proc = &read_hdd_power_status;
		hdd_power_status_file[i]->write_proc = &set_hdd_power;
		hdd_power_status_file[i]->data = (void *)&matsu_hdd_info[i];
	}

	goto INIT_OK;

HDD_POWER_STATUS_FILE_FAIL:
	for (i = 0; i < MAX_SUPPORTED_DISKS; i++)
	{
		sprintf(buf, "hdd%d", i);
		remove_proc_entry(buf, hdd_power_dir);
	}
	remove_proc_entry(HDD_POWER_DIR, NULL);

HDD_POWER_DIR_FAIL:
HDD_PRESENT_STATUS_FILE_FAIL:
	for (i = 0; i < MAX_SUPPORTED_DISKS; i++)
	{
		sprintf(buf, "sata%d", i);
		remove_proc_entry(buf, hdd_present_dir);
	}

	remove_proc_entry(HDD_PRESENT_DIR, NULL);

PRESENT_DIR_FAIL:
	remove_proc_entry(MODULE_DIR, NULL);

MODULE_DIR_FAIL:
	retval = -ENOMEM;

INIT_OK:
	return retval;
}


/*******************************************************************************
* function: hdd_ctrl_init
* argument:
* retvalue:
*******************************************************************************/
static int __init hdd_ctrl_init(void)
{
	printk(KERN_ALERT "[%s init proc]\n", HDD_CTRL_MODULE_NAME);

	/* create files within proc filesystem */
	if (init_proc_files()) return -ENOMEM;

	/* everything initialized */
	printk(KERN_INFO "%s %s initialized...\n",HDD_CTRL_MODULE_NAME, HDD_CTRL_MODULE_VERSION);
	return 0;
}
/*******************************************************************************
* function: hdd_ctrl_exit
* argument:
* retvalue:
*******************************************************************************/
static void __exit hdd_ctrl_exit(void)
{
	printk(KERN_ALERT "[%s exit proc]\n", HDD_CTRL_MODULE_NAME);
	int i = 0;
	char buf[32];

	for (i = 0; i < MAX_SUPPORTED_DISKS; i++)
	{
		sprintf(buf, "hdd%d", i);
		remove_proc_entry(buf, hdd_power_dir);
	}
	remove_proc_entry(HDD_POWER_DIR, NULL);
	for (i = 0; i < MAX_SUPPORTED_DISKS; i++)
	{
		sprintf(buf, "sata%d", i);
		remove_proc_entry(buf, hdd_present_dir);
	}
	remove_proc_entry(HDD_PRESENT_DIR, NULL);
	remove_proc_entry(HDD_POWER_DIR, NULL);
	remove_proc_entry(MODULE_DIR, NULL);
	printk(KERN_ALERT "hdd ctrl exit successfully....\n");
}

module_init (hdd_ctrl_init);
module_exit (hdd_ctrl_exit);

MODULE_LICENSE("GPL");
