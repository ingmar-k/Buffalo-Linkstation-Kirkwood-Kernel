/*****************************************************************************+
 DESCRIPTION:  Matsu Fan driver (sample)
 AUTHOR:       Wayne Lai
 DATE STARTED: July, 2010
 PROJECT:      WS-QX2(Matsu)

 Rev 0.2   July 28 2010
 - Initial revision.
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

#include "fan_ctrl.h"

//IT8721 registers section
#define	IT87_EC_ADD_REG		0xA15
#define	IT87_EC_DATA_REG	0xA16
// ITE8721 EC FAN REGISTERS
#define IT87_EC_FAN_MAIN_CTL	0x13
#define IT87_EC_FAN_CTL		0x14
#define IT87_EC_FAN_PWM1	0x15
#define IT87_EC_FAN_PWM2	0x16
#define IT87_EC_FAN_PWM3	0x17

#define IT87_EC_FAN_SMART_PWM1	0x63
#define IT87_EC_FAN_SMART_PWM2	0x6B
#define IT87_EC_FAN_SMART_PWM3	0x73

// ITE8721 EC FAN TACO REGISTERS
#define IT87_EC_FAN_TACO1	0x0D
#define IT87_EC_FAN_TACO2	0x0E
#define IT87_EC_FAN_TACO3	0x0F
#define IT87_EC_FAN_TACO4	0x80
#define IT87_EC_FAN_TACO1_EXT	0x18
#define IT87_EC_FAN_TACO2_EXT	0x19
#define IT87_EC_FAN_TACO3_EXT	0x1A
#define IT87_EC_FAN_TACO4_EXT	0x81

// ITE8721 EC TEMPERATURE SENSOR
#define IT87_EC_TMPIN1		0x29
#define IT87_EC_TMPIN2		0x2A
#define IT87_EC_TMPIN3		0x2B

//Common section
#define DRVVER			125		//device number
#define	DRVNAME			"matsu_fan"	//device name

//#define DEBUG
#if defined(DEBUG)
#define DPRINTK(fmt, args...)	printk(fmt, ##args)
#else
#define DPRINTK(fmt, atgs...)
#endif

int fan_major = DRVVER;
char *fan_devname = DRVNAME;
static unsigned int fan_threshold = 100;
static unsigned char fan_stop_speed = 0;
static unsigned char fan_slow_speed = 150;
static unsigned char fan_fast_speed = 200;
static unsigned char fan_full_speed = 255;
static unsigned int flag_show_speed_by_string = 1;

struct class *matsu_cls;

struct fan_info_st {
	uint8_t reg;
	uint8_t reg_ext;
	uint8_t reg_smart_pwm;
	uint8_t reg_pwm;
	uint8_t offset;
} fan_info[] = {
	{IT87_EC_FAN_TACO1, IT87_EC_FAN_TACO1_EXT, IT87_EC_FAN_SMART_PWM1, IT87_EC_FAN_PWM1, 0x1},
	{IT87_EC_FAN_TACO2, IT87_EC_FAN_TACO2_EXT, IT87_EC_FAN_SMART_PWM2, IT87_EC_FAN_PWM2, 0x2},
	{IT87_EC_FAN_TACO3, IT87_EC_FAN_TACO3_EXT, IT87_EC_FAN_SMART_PWM3, IT87_EC_FAN_PWM3, 0x4},
	{IT87_EC_FAN_TACO4, IT87_EC_FAN_TACO4_EXT, 0, 0, 0},
};

struct temp_info_st {
	uint8_t reg;
} temp_info[] = {
	{IT87_EC_TMPIN1},
	{IT87_EC_TMPIN2},
	{IT87_EC_TMPIN3},
};

/* Open the device */
int fan_open(struct inode *inode_ptr, struct file *fptr)
{
	DPRINTK("Fan driver is opened\n");                    
	return(0);
}

/* Close the device */
int fan_close(struct inode *inode_ptr, struct file *fptr)
{
	DPRINTK("Fan driver is closed\n");
	return(0);
}

unsigned int get_fan_rpm(unsigned char fan_num)
{
	unsigned char	read_value, read_value_ext;
	unsigned int	ret = 0;
	unsigned int	retry_max = 5;

	for(retry_max = 5; retry_max > 0; retry_max--)
	{
		outb(fan_info[fan_num].reg, IT87_EC_ADD_REG);
		read_value = inb(IT87_EC_DATA_REG);
		outb(fan_info[fan_num].reg_ext, IT87_EC_ADD_REG);
		read_value_ext = inb(IT87_EC_DATA_REG);
		if(read_value_ext != 0 || read_value != 0)
			ret = 1350000/(read_value_ext*256+read_value)/2;
		else
			ret = 0;

		if(ret <= 16500 && ret > 0)
			break;
		else
			printk("read_value_ext=%d : read_value=%d\n", read_value_ext, read_value);
		msleep(100);
	}

	DPRINTK("FAN%d speed is %d\n", fan_num + 1, ret);
	if(ret < fan_threshold)
		ret = 0;
	return ret;
}

unsigned int get_temp(unsigned char temp_num)
{
	unsigned char	read_value;
	unsigned int	ret = 0;

	outb(temp_info[temp_num].reg, IT87_EC_ADD_REG);
	read_value = inb(IT87_EC_DATA_REG);
	DPRINTK("TEMP%d is %d\n", temp_num + 1, read_value);
	ret = read_value;
	return ret;
}

unsigned int set_fan_speed(unsigned char fan_num, unsigned char set_val)
{
	unsigned int	write_data;
	unsigned char	read_value;

	if(fan_info[fan_num].reg_smart_pwm == 0 ||
		fan_info[fan_num].reg_pwm == 0)
	{
		return -EINVAL;
	}

	outb(IT87_EC_FAN_MAIN_CTL, IT87_EC_ADD_REG);
	read_value = inb(IT87_EC_DATA_REG);
	read_value |= fan_info[fan_num].offset;
	outb(read_value, IT87_EC_DATA_REG);

	write_data = set_val;
	outb(fan_info[fan_num].reg_smart_pwm, IT87_EC_ADD_REG);
	outb(write_data, IT87_EC_DATA_REG);

	outb(fan_info[fan_num].reg_pwm, IT87_EC_ADD_REG);
	read_value = inb(IT87_EC_DATA_REG);
	read_value &= 0x7F;
	outb(read_value, IT87_EC_DATA_REG);

	DPRINTK("set FAN%d speed %d\n", fan_num + 1, write_data);
	return 0;
}

unsigned int get_fan_speed(unsigned char fan_num)
{
	unsigned char	read_value;

	outb(IT87_EC_FAN_MAIN_CTL, IT87_EC_ADD_REG);
	read_value = inb(IT87_EC_DATA_REG);
	read_value |= fan_info[fan_num].offset;
	outb(read_value, IT87_EC_DATA_REG);

	outb(fan_info[fan_num].reg_smart_pwm, IT87_EC_ADD_REG);
	read_value = inb(IT87_EC_DATA_REG);

	return read_value;
}

// IOCTL interface for user to get and set
int fan_ioctl(struct inode * inode,struct file *file,unsigned int cmd, unsigned long para)
{
	unsigned int	ret = 0;
	unsigned char dev_id = (unsigned char)(cmd & ~IOCTL_CMD_MASK);

	switch(cmd & IOCTL_CMD_MASK)
	{
		case GET_TEMP_CMD_BASE:
			//interface to read temperature sensor 1
			//interface to read temperature sensor 2
			//interface to read temperature sensor 3
			if(dev_id > MAX_GET_TEMP)
				goto IOCTL_WRONG_CMD;

			ret = get_temp(dev_id);
			break;
		case GET_FAN_CMD_BASE:
			//interface to read FAN1 speed.(In 1U)
			//interface to read FAN2 speed.(In 1U)
			//interface to read FAN3 speed.(In Box)
			if(dev_id > MAX_GET_FAN)
				goto IOCTL_WRONG_CMD;

			ret = get_fan_rpm(dev_id);
			break;
		case SET_FAN_CMD_BASE:
			//interface to set FAN1 speed.(In 1U)
			//interface to set FAN2 speed.(In 1U)
			//interface to set FAN3 speed.(In Box)
			//interface to read FAN4 speed.(In Box)
			if(dev_id > MAX_SET_FAN)
				goto IOCTL_WRONG_CMD;

			ret = set_fan_speed(dev_id, (unsigned char )para);
			break;
		default:
			goto IOCTL_WRONG_CMD;
			break;
	}		 
	return ret;

IOCTL_WRONG_CMD:
	printk("wrong command");
	return ret;
}

//file operations for this device.
struct file_operations fan_fops = {
	.open = fan_open,
	.ioctl = fan_ioctl,
	.release = fan_close,
	.owner = THIS_MODULE,
};

struct class *MatsuClassGet(void)
{
	struct class *tmp_cls;

	if(matsu_cls)
		return matsu_cls;

	tmp_cls = class_create(THIS_MODULE, "matsu");
	if(IS_ERR(tmp_cls))
	{
		printk("%s : class_create failed.", __FUNCTION__);
		return NULL;
	}

	matsu_cls = tmp_cls;
	return matsu_cls;
}

static ssize_t show_dev_number(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d:%d\n", fan_major, 0);
}
DEVICE_ATTR(matsu_fan_dev, 0444, show_dev_number, NULL);

static ssize_t show_fan1_rpm(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", get_fan_rpm(0));
}

static ssize_t show_fan2_rpm(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", get_fan_rpm(1));
}

static ssize_t show_fan3_rpm(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", get_fan_rpm(2));
}

static ssize_t show_fan4_rpm(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", get_fan_rpm(3));
}
DEVICE_ATTR(fan1_rpm, 0444, show_fan1_rpm, NULL);
DEVICE_ATTR(fan2_rpm, 0444, show_fan2_rpm, NULL);
DEVICE_ATTR(fan3_rpm, 0444, show_fan3_rpm, NULL);
DEVICE_ATTR(fan4_rpm, 0444, show_fan4_rpm, NULL);

static ssize_t show_temp1(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", get_temp(0));
}

static ssize_t show_temp2(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", get_temp(1));
}

static ssize_t show_temp3(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", get_temp(2));
}
DEVICE_ATTR(temp1, 0444, show_temp1, NULL);
DEVICE_ATTR(temp2, 0444, show_temp2, NULL);
DEVICE_ATTR(temp3, 0444, show_temp3, NULL);

static unsigned long fan_speed_str_to_ul(const char *buf)
{
	if((strncmp(buf, "stop", strlen("stop")) == 0))
		return fan_stop_speed;
	else if((strncmp(buf, "slow", strlen("slow")) == 0))
		return fan_slow_speed;
	else if((strncmp(buf, "fast", strlen("fast")) == 0))
		return fan_fast_speed;
	else if((strncmp(buf, "full", strlen("full")) == 0))
		return fan_full_speed;

	return simple_strtoul(buf, NULL, 10);
}

static unsigned long fan_speed_ul_to_str(const char *buf, size_t len, unsigned long speed)
{
	if(speed <= fan_stop_speed)
		snprintf(buf, len, "stop");
	else if(speed > fan_stop_speed && speed <= fan_slow_speed)
		snprintf(buf, len, "slow");
	else if(speed > fan_slow_speed && speed <= fan_fast_speed)
		snprintf(buf, len, "fast");
	else if(speed > fan_fast_speed)
		snprintf(buf, len, "full");
	else
		snprintf(buf, len, "Unknown");
}

static ssize_t show_fan1_speed(struct device *dev, struct device_attribute *attr, char *buf)
{
	if(flag_show_speed_by_string == 1)
	{
		char szSpeed[32];
		fan_speed_ul_to_str(szSpeed, sizeof(szSpeed), get_fan_speed(0));
		sprintf(buf, "%s\n", szSpeed);
	}
	else
		return sprintf(buf, "%d\n", get_fan_speed(0));
}

static ssize_t store_fan1_speed(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned long write_val = fan_speed_str_to_ul((const char*) buf );
	if(write_val > 0xff)
		return -EINVAL;

	set_fan_speed(0, (unsigned char)(write_val & 0xff));
	return count;
}

static ssize_t show_fan2_speed(struct device *dev, struct device_attribute *attr, char *buf)
{
	if(flag_show_speed_by_string == 1)
	{
		char szSpeed[32];
		fan_speed_ul_to_str(szSpeed, sizeof(szSpeed), get_fan_speed(1));
		sprintf(buf, "%s\n", szSpeed);
	}
	else
		return sprintf(buf, "%d\n", get_fan_speed(1));
}

static ssize_t store_fan2_speed(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned long write_val = fan_speed_str_to_ul((const char*) buf );
	if(write_val > 0xff)
		return -EINVAL;

	set_fan_speed(1, (unsigned char)(write_val & 0xff));
	return count;
}

static ssize_t show_fan3_speed(struct device *dev, struct device_attribute *attr, char *buf)
{
	if(flag_show_speed_by_string == 1)
	{
		char szSpeed[32];
		fan_speed_ul_to_str(szSpeed, sizeof(szSpeed), get_fan_speed(2));
		sprintf(buf, "%s\n", szSpeed);
	}
	else
		return sprintf(buf, "%d\n", get_fan_speed(2));
}

static ssize_t store_fan3_speed(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned long write_val = fan_speed_str_to_ul((const char*) buf );
	if(write_val > 0xff)
		return -EINVAL;

	set_fan_speed(2, (unsigned char)(write_val & 0xff));
	return count;
}

static ssize_t show_fan4_speed(struct device *dev, struct device_attribute *attr, char *buf)
{
	if(flag_show_speed_by_string == 1)
	{
		char szSpeed[32];
		fan_speed_ul_to_str(szSpeed, sizeof(szSpeed), get_fan_speed(3));
		sprintf(buf, "%s\n", szSpeed);
	}
	else
		return sprintf(buf, "%d\n", get_fan_speed(3));
}

static ssize_t store_fan4_speed(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned long write_val = fan_speed_str_to_ul((const char*) buf );
	
	if(write_val > 0xff)
		return -EINVAL;

	set_fan_speed(3, (unsigned char)(write_val & 0xff));
	return count;
}

static ssize_t show_show_speed_by_string(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", flag_show_speed_by_string);
}

static ssize_t store_show_speed_by_string(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned long value = simple_strtoul(buf, NULL, 10);
	
	if(value != 0 && value != 1)
		return -EINVAL;

	flag_show_speed_by_string = value;

	return count;
}

DEVICE_ATTR(fan1_speed, 0644, show_fan1_speed, store_fan1_speed);
DEVICE_ATTR(fan2_speed, 0644, show_fan2_speed, store_fan2_speed);
DEVICE_ATTR(fan3_speed, 0644, show_fan3_speed, store_fan3_speed);
DEVICE_ATTR(fan4_speed, 0644, show_fan4_speed, store_fan4_speed);
DEVICE_ATTR(show_speed_by_string, 0644, show_show_speed_by_string, store_show_speed_by_string);

static ssize_t show_fan_threshold(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", fan_threshold);
}

static ssize_t store_fan_threshold(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	fan_threshold = simple_strtoul(buf, NULL, 10);
	return count;
}
DEVICE_ATTR(fan_threshold, 0644, show_fan_threshold, store_fan_threshold);

static ssize_t show_fan_stop_speed(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", fan_stop_speed);
}

static ssize_t store_fan_stop_speed(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned long tmp_speed = simple_strtoul(buf, NULL, 10);
	if(tmp_speed > 0xff)
		return -EINVAL;

	fan_stop_speed = tmp_speed;
	return count;
}

static ssize_t show_fan_slow_speed(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", fan_slow_speed);
}

static ssize_t store_fan_slow_speed(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned long tmp_speed = simple_strtoul(buf, NULL, 10);
	if(tmp_speed > 0xff)
		return -EINVAL;

	fan_slow_speed = tmp_speed;
	return count;
}
static ssize_t show_fan_fast_speed(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", fan_fast_speed);
}

static ssize_t store_fan_fast_speed(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned long tmp_speed = simple_strtoul(buf, NULL, 10);
	if(tmp_speed > 0xff)
		return -EINVAL;

	fan_fast_speed = tmp_speed;
	return count;
}

static ssize_t show_fan_full_speed(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", fan_full_speed);
}

static ssize_t store_fan_full_speed(struct device *dev, struct device_attribute *attr, char *buf, size_t count)
{
	unsigned long tmp_speed = simple_strtoul(buf, NULL, 10);
	if(tmp_speed > 0xff)
		return -EINVAL;

	fan_full_speed = tmp_speed;
	return count;
}
DEVICE_ATTR(fan_stop_speed, 0644, show_fan_stop_speed, store_fan_stop_speed);
DEVICE_ATTR(fan_slow_speed, 0644, show_fan_slow_speed, store_fan_slow_speed);
DEVICE_ATTR(fan_fast_speed, 0644, show_fan_fast_speed, store_fan_fast_speed);
DEVICE_ATTR(fan_full_speed, 0644, show_fan_full_speed, store_fan_full_speed);

//register the device to kernel.
static int __init matsufan_init(void)
{
	int ret;
	struct class *cls;

	printk("enter fan module init\n");

	/* Register device, and may be allocating major# */
	ret = register_chrdev(fan_major, fan_devname, &fan_fops);
	if(ret <0)
	{
		printk("%s cannot register\n", fan_devname);
		return(ret);
	}
	else
	{
		if(fan_major==0)
			fan_major = ret;
	}

	cls = MatsuClassGet();
	if(cls)
	{
		struct device *dev = device_create(cls, NULL, 0, NULL, "fan");
		int rv = 0;
		rv = device_create_file(dev, &dev_attr_matsu_fan_dev);
		rv = device_create_file(dev, &dev_attr_fan1_rpm);
		rv = device_create_file(dev, &dev_attr_fan2_rpm);
		rv = device_create_file(dev, &dev_attr_fan3_rpm);
		rv = device_create_file(dev, &dev_attr_fan4_rpm);
		rv = device_create_file(dev, &dev_attr_temp1);
		rv = device_create_file(dev, &dev_attr_temp2);
		rv = device_create_file(dev, &dev_attr_temp3);
		rv = device_create_file(dev, &dev_attr_fan1_speed);
		rv = device_create_file(dev, &dev_attr_fan2_speed);
		rv = device_create_file(dev, &dev_attr_fan3_speed);
		rv = device_create_file(dev, &dev_attr_fan4_speed);
		rv = device_create_file(dev, &dev_attr_fan_threshold);
		rv = device_create_file(dev, &dev_attr_fan_stop_speed);
		rv = device_create_file(dev, &dev_attr_fan_slow_speed);
		rv = device_create_file(dev, &dev_attr_fan_fast_speed);
		rv = device_create_file(dev, &dev_attr_fan_full_speed);
		rv = device_create_file(dev, &dev_attr_show_speed_by_string);
	}

	return 0;
}

//unregister the device from kernel.
static void __exit matsufan_exit_cleanup(void)
{
	
	printk(DRVNAME " MODULE HAS BEEN REMOVED!\n");
	unregister_chrdev(fan_major, fan_devname);
}

module_init(matsufan_init);
module_exit(matsufan_exit_cleanup);
MODULE_LICENSE("GPL");
