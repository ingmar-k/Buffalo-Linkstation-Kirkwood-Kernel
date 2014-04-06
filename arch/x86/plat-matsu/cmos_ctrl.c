/*****************************************************************************+
 DESCRIPTION:  Matsu CMOS driver (sample)
 AUTHOR:       Wayne Lai
 DATE STARTED: August, 2010
 PROJECT:      WS-QX2(Matsu)

 Rev 0.2   August 12 2010
 *******************************************************************************/
#include <asm/io.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <asm/delay.h>
#include <linux/sched.h>
#include <linux/string.h>
#include <linux/cdev.h>
#include <linux/device.h>

//I/O index
#define	CMOS_INDEX_PORT		0x72
#define	CMOS_DATA_PORT		0x73
#define	POWER_OFFSET		0xAA
#define	POWER_MASK		0xC0
#define	WOL_OFFSET		0xC2
#define	WOL_MASK		0x80

//IOCTL interface command
#define GET_POWER		1
#define SET_POWER		2
#define GET_WOL			3
#define SET_WOL			4

//Common section
#define DRVVER			124		//device number
#define	DRVNAME			"matsu_cmos"	//device name

int cmos_major = DRVVER;
char *cmos_devname = DRVNAME;

extern struct class *matsu_cls;
extern struct class *MatsuClassGet(void);

#define AC_POWER_LOSS_POWER_OFF		0
#define AC_POWER_LOSS_POWER_ON		1
#define AC_POWER_LOSS_LAST_STATE	2

#define WOL_ENABLE		0
#define WOL_DISABLE		1

struct matsu_cmos_power_value_to_str_st {
	unsigned char value;
	char str_value[16];
} matsu_cmos_power_value_to_str[] = {
	{AC_POWER_LOSS_POWER_OFF,	"off"},
	{AC_POWER_LOSS_POWER_ON,	"on"},
	{AC_POWER_LOSS_LAST_STATE,	"last_state"},
};
#define MAX_MATSU_CMOS_POWER_VALUE_TO_STR	(sizeof(matsu_cmos_power_value_to_str) / sizeof(matsu_cmos_power_value_to_str[0]))

struct matsu_cmos_wol_value_to_str_st {
	unsigned char value;
	char str_value[16];
} matsu_cmos_wol_value_to_str[] = {
	{WOL_ENABLE,	"enable"},
	{WOL_DISABLE,	"disable"},
};
#define MAX_MATSU_CMOS_WOL_VALUE_TO_STR		(sizeof(matsu_cmos_wol_value_to_str) / sizeof(matsu_cmos_wol_value_to_str[0]))

/* Open the device */
int cmos_open(struct inode *inode_ptr, struct file *fptr)
{
	printk("CMOS driver is opened\n");                    
	return(0);
}

/* Close the device */
int cmos_close(struct inode *inode_ptr, struct file *fptr)
{
	printk("CMOS driver is closed\n");
	return(0);
}

static unsigned int
matsu_cmos_get_power_setting(void)
{
	unsigned char	read_value;

	outb(POWER_OFFSET, CMOS_INDEX_PORT);
	read_value = inb(CMOS_DATA_PORT);
	read_value&= POWER_MASK;
	read_value = read_value >> 6;
	printk("AC Power Loss is %d\n",read_value);
	return (unsigned int)read_value;
}

static unsigned int
matsu_cmos_set_power_setting(unsigned long set_value)
{
	unsigned int	write_data;

	outb(POWER_OFFSET, CMOS_INDEX_PORT);
	write_data = (unsigned int)set_value;
	write_data = write_data << 6;
	outb(write_data, CMOS_DATA_PORT);
	write_data = write_data >> 6;
	printk("set AC Power Loss to %d\n",write_data);
	return 0;
}

static unsigned int
matsu_cmos_get_wol_setting(void)
{
	unsigned char	read_value;

	outb(WOL_OFFSET, CMOS_INDEX_PORT);
	read_value = inb(CMOS_DATA_PORT);
	read_value &= WOL_MASK;
	read_value = read_value >> 7;
	printk("Wake on LAN is %d\n",read_value);
	return (unsigned int)read_value;
}

static unsigned int
matsu_cmos_set_wol_setting(unsigned long set_value)
{
	unsigned int	write_data;

	outb(WOL_OFFSET, CMOS_INDEX_PORT);
	write_data = (unsigned int)set_value;
	write_data = write_data << 7;
	outb(write_data, CMOS_DATA_PORT);
	write_data = write_data >> 7;
	printk("set Wake on LAN to %d\n",write_data);
	return 0;
}

// IOCTL interface for user to get and set
int cmos_ioctl(struct inode * inode,struct file *file,unsigned int cmd, unsigned long para)
{
	unsigned int	ret = 0;

	switch(cmd)
	{
		//interface to read AC Power Loss in CMOS
		case GET_POWER:
			ret = matsu_cmos_get_power_setting();
			break;

		//interface to write AC Power Loss in CMOS
		case SET_POWER:
			ret = matsu_cmos_set_power_setting(para);
			break;

		//interface to read wake on LAN in CMOS
		case GET_WOL:
			ret = matsu_cmos_get_wol_setting();
			break;

		//interface to write wake on LAN in CMOS
		case SET_WOL:
			ret = matsu_cmos_set_wol_setting(para);
			break;

		default:
			printk("wrong command");
			break;
	}		 
	return ret;
}

//file operations for this device.
struct file_operations cmos_fops = {
open: cmos_open,
ioctl: cmos_ioctl,
release: cmos_close,
owner: THIS_MODULE,
};

static ssize_t show_dev_number(struct device *dev, struct device_attribute *attr, char *buf)
{
        return sprintf(buf, "%d:%d\n", cmos_major, 0);
}
DEVICE_ATTR(matsu_cmos_dev, 0444, show_dev_number, NULL);

static ssize_t show_power_setting(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int value = matsu_cmos_get_power_setting();
	char *str_value = NULL;
	int i = 0;

	for(i = 0; i < MAX_MATSU_CMOS_POWER_VALUE_TO_STR; i++)
	{
		if(value == matsu_cmos_power_value_to_str[i].value)
		{
			str_value = matsu_cmos_power_value_to_str[i].str_value;
			break;
		}
	}
	if(str_value == NULL)
	{
		printk("CMOS setting is broken\n");
		return 0;
	}

        return sprintf(buf, "%s\n", str_value);
}

static ssize_t store_power_setting(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned int i = 0;
        unsigned long tmp_value = (unsigned long)-1;

	for(i = 0; i < strlen(buf); i++)
	{
		if(buf[i] == '\n')
		{
			buf[i] = 0;
			break;
		}
	}

	for(i = 0; i < MAX_MATSU_CMOS_POWER_VALUE_TO_STR; i++)
	{
		if((strlen(buf) == strlen(matsu_cmos_power_value_to_str[i].str_value)) && (strncmp(buf, matsu_cmos_power_value_to_str[i].str_value, strlen(matsu_cmos_power_value_to_str[i].str_value)) == 0))
		{
			tmp_value = matsu_cmos_power_value_to_str[i].value;
			break;
		}
	}
        if(tmp_value  == (unsigned long)-1)
                return -EINVAL;

	matsu_cmos_set_power_setting(tmp_value);
        return count;
}
DEVICE_ATTR(power_setting, 0644, show_power_setting, store_power_setting);

static ssize_t show_wol_setting(struct device *dev, struct device_attribute *attr, char *buf)
{
	unsigned int value = matsu_cmos_get_wol_setting();
	char *str_value = NULL;
	int i = 0;

	for(i = 0; i < MAX_MATSU_CMOS_WOL_VALUE_TO_STR; i++)
	{
		if(value == matsu_cmos_wol_value_to_str[i].value)
		{
			str_value = matsu_cmos_wol_value_to_str[i].str_value;
		}
	}
	if(str_value == NULL)
	{
		printk("CMOS setting is broken\n");
		return 0;
	}
        return sprintf(buf, "%s\n", str_value);
}

static ssize_t store_wol_setting(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned int i = 0;
        unsigned long tmp_value = (unsigned long)-1;

	for(i = 0; i < strlen(buf); i++)
	{
		if(buf[i] == '\n')
		{
			buf[i] = 0;
			break;
		}
	}

	for(i = 0; i < MAX_MATSU_CMOS_WOL_VALUE_TO_STR; i++)
	{
		if((strlen(buf) == strlen(matsu_cmos_wol_value_to_str[i].str_value)) && (strncmp(buf, matsu_cmos_wol_value_to_str[i].str_value, strlen(matsu_cmos_wol_value_to_str[i].str_value)) == 0))
		{
			tmp_value = matsu_cmos_wol_value_to_str[i].value;
			break;
		}
	}
        if(tmp_value == (unsigned long)-1)
                return -EINVAL;

	matsu_cmos_set_wol_setting(tmp_value);
        return count;
}
DEVICE_ATTR(wol_setting, 0644, show_wol_setting, store_wol_setting);

//register the device to kernel.
static int __init matsucmos_init(void)
{
	int ret;
	struct class *cls;

	printk("enter cmos module init\n");

	/* Register device, and may be allocating major# */
	ret = register_chrdev(cmos_major, cmos_devname, &cmos_fops);
	if(ret <0)
	{
		printk("%s cannot register\n", cmos_devname);
		return(ret);
	}
	else
	{
		if(cmos_major==0)
			cmos_major = ret;
	}

	cls = MatsuClassGet();
	if(cls)
	{
		struct device *dev = device_create(cls, NULL, 0, NULL, "cmos");
		int rv = 0;
		rv = device_create_file(dev, &dev_attr_matsu_cmos_dev);
		rv = device_create_file(dev, &dev_attr_power_setting);
		rv = device_create_file(dev, &dev_attr_wol_setting);
	}

	return 0;
}

//unregister the device from kernel.
static void __exit matsucmos_exit_cleanup(void)
{
	
	printk(DRVNAME " CMOS MODULE HAS BEEN REMOVED!\n");
	unregister_chrdev(cmos_major, cmos_devname);
}

module_init(matsucmos_init);
module_exit(matsucmos_exit_cleanup);
MODULE_LICENSE("GPL");
