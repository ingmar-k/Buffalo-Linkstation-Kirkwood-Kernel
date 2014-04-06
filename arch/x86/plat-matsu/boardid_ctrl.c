/*****************************************************************************+
 DESCRIPTION:  Matsu ID driver (sample)
 AUTHOR:       Wayne Lai
 DATE STARTED: August, 2010
 PROJECT:      WS-QX2(Matsu)

 Rev 0.1   August 18 2010
 *******************************************************************************/
#include <asm/io.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <asm/delay.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/cdev.h>

/* Define Vendor and product id for ich9r */
#define INTEL_ICH9R_VENDOR_ID	0x8086
#define INTEL_ICH9R_DEVICE_ID	0x2916

#define GPIO_BAR_OFFSET		0x48
#define GP_LVL			0x0c

/* Define GPIO
      GPIO31   GPIO30   GPIO29        SKU
      ------   ------   ------     ------------
(1)      0       0        0         2HDD box
(2)      0       0        1         4HDD box
(3)      0       1        0         6HDD box
(4)      0       1        1         8HDD box
(5)      1       0        0         1U

*/
#define GPIO29			0x20000000	/*GPIO29*/
#define GPIO30			0x40000000	/*GPIO30*/
#define GPIO31			0x80000000	/*GPIO31*/
#define BOARD_ID_MASK		(GPIO29 | GPIO30 | GPIO31)
#define BOARD_ID_OFFSET		29

#define	GET_ID			1

//Common section
#define DRVVER			123		//device number
#define	DRVNAME			"matsu_id"	//device name

//#define DEBUG
#if defined(DEBUG)
	#define debug(fmt, args...)	printk(fmt, ##args)
#else
	#define debug(fmt, args...)
#endif

int id_major = DRVVER;
char *id_devname = DRVNAME;

extern struct class *matsu_cls;
extern struct class *MatsuClassGet(void);

static unsigned int ich9r_gpio_base_addr;

static int find_ich9_gpio_base_addr(void)
{
	struct pci_dev *pdev = NULL;

	pdev = pci_get_device(INTEL_ICH9R_VENDOR_ID, INTEL_ICH9R_DEVICE_ID, NULL);
	if (!pdev) return -ENODEV;
	pci_read_config_dword(pdev, GPIO_BAR_OFFSET, &ich9r_gpio_base_addr);
	ich9r_gpio_base_addr &= 0x0000ff80;

	return 0;
}

/* Open the device */
int id_open(struct inode *inode_ptr, struct file *fptr)
{
	printk("id driver is opened\n");                    
	return(0);
}

/* Close the device */
int id_close(struct inode *inode_ptr, struct file *fptr)
{
	printk("id driver is closed\n");
	return(0);
}

unsigned int
GetBoardId(void)
{
	unsigned int	read_value;
	read_value = (inl(ich9r_gpio_base_addr + GP_LVL) & BOARD_ID_MASK) >> BOARD_ID_OFFSET;

	debug("gpio31 is %d \n", (inl(ich9r_gpio_base_addr + GP_LVL)&GPIO31)>>31 );
	debug("gpio30 is %d \n", (inl(ich9r_gpio_base_addr + GP_LVL)&GPIO30)>>30 );
	debug("gpio29 is %d \n", (inl(ich9r_gpio_base_addr + GP_LVL)&GPIO29)>>29 );
	debug("id is %d \n",read_value);

	return read_value;
}

// IOCTL interface for user to get
int id_ioctl(struct inode * inode,struct file *file,unsigned int cmd, unsigned long para)
{
	unsigned int	ret = -1;

	switch(cmd)
	{
		//interface to read id
		case GET_ID:
			ret = GetBoardId();
			break;

		default:
			printk("wrong command");
			ret = -1;
			break;
	}		 
	return ret;
}

//file operations for this device.
struct file_operations id_fops = {
open: id_open,
ioctl: id_ioctl,
release: id_close,
owner: THIS_MODULE,
};

static ssize_t show_dev_number(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d:%d\n", id_major, 0);
}
DEVICE_ATTR(matsu_board_dev, 0444, show_dev_number, NULL);

static ssize_t show_matsu_board_id(struct device *dev, struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%d\n", GetBoardId());
}
DEVICE_ATTR(id, 0444, show_matsu_board_id, NULL);

//register the device to kernel.
static int __init matsuid_init(void)
{
	int ret = -1;
	struct class *cls;

	debug("enter id module init\n");

	/* find intel ich9 device */
	if (find_ich9_gpio_base_addr()) return -ENODEV;

	/* Register device, and may be allocating major# */
	ret = register_chrdev(id_major, id_devname, &id_fops);
	if(ret <0)
	{
		printk("%s cannot register\n", id_devname);
		return(ret);
	}
	else
	{
		if(id_major==0)
			id_major = ret;
	}

	cls = MatsuClassGet();
	if(cls)
	{
		struct device *dev = device_create(cls, NULL, 0, NULL, "board");
		int rv = 0;
		rv = device_create_file(dev, &dev_attr_matsu_board_dev);
		rv = device_create_file(dev, &dev_attr_id);
	}

	return 0;
}

//unregister the device from kernel.
static void __exit matsuid_exit_cleanup(void)
{
	
	printk(DRVNAME " ID MODULE HAS BEEN REMOVED!\n");
	unregister_chrdev(id_major, id_devname);
}

module_init(matsuid_init);
module_exit(matsuid_exit_cleanup);
MODULE_LICENSE("GPL");
