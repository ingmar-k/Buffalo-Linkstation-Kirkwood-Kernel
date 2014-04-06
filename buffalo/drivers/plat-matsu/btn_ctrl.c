/*****************************************************************************+
 DESCRIPTION:  Matsu button driver (sample)
 AUTHOR:       Ian Juang
 DATE STARTED: July, 2010
 PROJECT:      WS-QX2(Matsu)

 Rev 0.3   Aug  05 2010
 - Adding an "if" clause in the interrupt handler to check for the interrupt 
   source(PBD's IRQ).
 - Indicate the timer source.
*******************************************************************************
 Rev 0.2   Aug  04 2010
 - Modify button interrupt event to become triggered by IT87 de-bounce IRQ.
 Rev 0.1   July 26 2010
 - Initial revision.
--------------------------------------------------------------------------------
 CONVENTIONS:

+----------------------------------------------------------------------------+*/
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <asm/io.h>

#include <linux/timer.h>
#include <asm/param.h>

//Common section
#define DRVVER			"0.3"
#define	DRVNAME			"matsu_btn"

#if defined(CONFIG_BUFFALO_PLATFORM)
#include "buffalo/kernevnt.h"
#endif

//IT8721 registers section
#define CONFIG_ADDR_PORT	0x2E
#define CONFIG_DATA_PORT	0x2F

#define IT87_LDN_ADDR		0x07
#define	IT87_GPIO_LDN		0x07

#define IT87_GP34_LOC		0x1C
#define IT87_GP35_LOC		0x1D
#define	IT87_IRQEN		0x40

#define IT87_REG_PNL_DBOUNCE0	0xE0
#define IT87_REG_PNL_DBOUNCE1	0xE1
#define IT87_REG_PNL_IRQ_SEL	0x70
#define IT87_PNL_IRQ3		0x03	//IRQ level 3
#define IT87_REG_SMI_STS2	0xF3

#define IT87_SMI_DEB_MASK	0xC0
#define IT87_DEB0		0x80	//bit7
#define IT87_DEB1		0x40	//bit6
#define IT87_DEB0DEB1		0xC0	//bits[6-7]
#define IT87_SMI_STS2_PBD	0x01	//bit0 PBD's IRQ

#define IT87_GP34_BIT4		0x10
#define IT87_GP35_BIT5		0x20
//Simple I/O base addr(Index62h and 63h)+ GPIO Set2
#define IT87_SIMPLE_IO_SET2	0xA22

//Buttons section
#define BTN_DISP	    	   1
#define BTN_FUNC	 	   2
      
#define TIMER_DELAY		(HZ/500)//this value can be adjusted
static struct timer_list btn_timer;

#define MODULE_DIR		"buffalo/gpio"
#define SWITCH_DIR		"buffalo/gpio/switch"

static struct button_info_st {
	char name[32];
	unsigned char reg_stat;
} button_info[] = {
	{"func", IT87_DEB1},
	{"display", IT87_DEB0},
};
#define NUM_SUPPORTED_BUTTON_NUM	(sizeof(button_info) / sizeof(button_info[0]))

/*+--------------------------------------------+
This procedure is for program LPC configuration 
registers. Entering and exiting MBPnP mode.
+--------------------------------------------+*/
void EnterMBPnP(void)
{
	outb(0x87, CONFIG_ADDR_PORT);
	outb(0x01, CONFIG_ADDR_PORT);
	outb(0x55, CONFIG_ADDR_PORT);
	outb(0x55, CONFIG_ADDR_PORT);
}
void ExitMBPnP(void)
{
	outb(0x02, CONFIG_ADDR_PORT);
	outb(0x02, CONFIG_DATA_PORT);		
}

/*+--------------------------------------------+
This procedure is for getting the GPIO status of
buttons.

Returns:
+--------------------------------------------+*/
static int it87_gpio_getsts(int pin)
{
	int	off;
	if(pin == BTN_DISP){
		off = IT87_GP35_BIT5;	//pin number of DISP button
	}else{
		off = IT87_GP34_BIT4;	//pin number of FUNC button
	}		

	return inb(IT87_SIMPLE_IO_SET2) & off;
}

/*+--------------------------------------------+
A timer for checking if the de-bounce gpio pin
has been released. This interrupt won't be 
triggered again until the IT87_REG_SMI_STS2 is
cleared by software.

Returns: 
+--------------------------------------------+*/
static void timer_function(unsigned long data)
{
	int	pin_sts;

	pin_sts=it87_gpio_getsts(data);
	
	if(pin_sts){
	//Clear de-bounce ping SMI status
		EnterMBPnP();
		outb(IT87_REG_SMI_STS2, CONFIG_ADDR_PORT);
		outb(IT87_DEB0DEB1 | IT87_SMI_STS2_PBD, CONFIG_DATA_PORT);		
		ExitMBPnP();
	//Delete timer
		del_timer(&btn_timer);
		printk(KERN_ALERT "Timer %ld is cleared!\n",data);
	}else{
        	mod_timer(&btn_timer, jiffies + TIMER_DELAY);
	}
}

/*+--------------------------------------------+
Matsu button interrupt handler

Handling display and funcion button irq demand. 
Check IT8721 SMI# Status Register2 to determine 
which button was pressed.

Returns: 
+--------------------------------------------+*/
static irqreturn_t matsu_interrupt(int irq, void *dev_id)
{
	int	pin=0;
	unsigned char smi_sts;
#if defined(CONFIG_BUFFALO_PLATFORM)
	char msg[32];
	memset(msg, 0, sizeof(msg));
#endif
	//Get SMI status of two de-bounce pin
	EnterMBPnP();	
	outb(IT87_REG_SMI_STS2, CONFIG_ADDR_PORT);

	smi_sts = inb(CONFIG_DATA_PORT);
	ExitMBPnP();
	//Check if the interrupt source is PBD's IRQ
	if(smi_sts & IT87_SMI_STS2_PBD)
	{
		smi_sts &= IT87_SMI_DEB_MASK;
		//Check which button was pressed
		if (smi_sts == IT87_DEB0){
#if defined(CONFIG_BUFFALO_PLATFORM)
			sprintf(msg, "micon_interrupts");
#else
			printk(KERN_ALERT "(1)The DISPLAY button was pressed\n");
#endif
			pin = BTN_DISP;
		}
		else if (smi_sts == IT87_DEB1){
#if defined(CONFIG_BUFFALO_PLATFORM)
			sprintf(msg, "micon_interrupts");
#else
			printk(KERN_ALERT "(2)The FUNCTION button was pressed\n");
#endif
			pin = BTN_FUNC;
		}
		//Setup timer for checking button gpio state
 		init_timer(&btn_timer);

		btn_timer.expires = jiffies + TIMER_DELAY;
		btn_timer.data = pin;
		btn_timer.function = timer_function;
	
		add_timer(&btn_timer);
#if defined(CONFIG_BUFFALO_PLATFORM)
		if(msg[0] != '\0')
			buffalo_kernevnt_queuein(msg);
#endif
	}
	return IRQ_HANDLED;
}

static int
read_button_status(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;
	struct button_info_st *button_info = (struct button_info_st *)data;
	unsigned char smi_sts;

	EnterMBPnP();

	//Get SMI status of two de-bounce pin
	outb(IT87_REG_SMI_STS2, CONFIG_ADDR_PORT);
	smi_sts = inb(CONFIG_DATA_PORT);
	smi_sts &= IT87_SMI_DEB_MASK;

	ExitMBPnP();

	//Check which button was pressed
	if ((smi_sts & button_info->reg_stat))
		len = sprintf(page, "on\n");
	else
		len = sprintf(page, "off\n");

	if(off > 0){
		*eof = 1;
		return 0;
	}

	*eof = 1;
	return len;
}

/*+--------------------------------------------+
Initial procedure, device driver interrupt 
handler register IRQ3 and initialize the SIO(IT 
8721) GPIO34/35.

Returns:
+--------------------------------------------+*/
static int __init matsubtn_init(void)
{
	int ret;

        ret = request_irq(3, matsu_interrupt, IRQF_SHARED, DRVNAME, matsu_interrupt);
    	if (ret){
    		printk (KERN_ALERT "!!!!! Error requesting irq 3: returned %d\n", ret);
		free_irq(3, 0);
        	return -EBUSY;
    	}
 
	//Enter ITE config
	EnterMBPnP();
	
	//Select LDN 07h	
	outb(IT87_LDN_ADDR, CONFIG_ADDR_PORT);
	outb(IT87_GPIO_LDN, CONFIG_DATA_PORT);
	
	//Set GPIO34/35 as De-bounce GPIO
	outb(IT87_REG_PNL_DBOUNCE0, CONFIG_ADDR_PORT);
	outb(IT87_GP34_LOC | IT87_IRQEN, CONFIG_DATA_PORT);
	outb(IT87_REG_PNL_DBOUNCE1, CONFIG_ADDR_PORT);
	outb(IT87_GP35_LOC, CONFIG_DATA_PORT);
	
	outb(IT87_REG_PNL_IRQ_SEL, CONFIG_ADDR_PORT);
	outb(IT87_PNL_IRQ3, CONFIG_DATA_PORT);
	//Exit ITE config
	ExitMBPnP();

#if defined(CONFIG_BUFFALO_PLATFORM)
	//static struct proc_dir_entry *btn_ctrl_dir;
	static struct proc_dir_entry *switch_dir;
	static struct proc_dir_entry *switch_entry;
	int i = 0;

/*
	btn_ctrl_dir = proc_mkdir(MODULE_DIR, NULL);
	if(btn_ctrl_dir == NULL)
	{
		printk(KERN_ALERT "%s directory create failed.....\n", MODULE_DIR);
		goto MODULE_DIR_FAIL;
	}
	printk(KERN_ALERT "%s directory create successful.....\n", MODULE_DIR);
*/
	switch_dir = proc_mkdir(SWITCH_DIR, NULL);
	if(switch_dir == NULL)
	{
		printk(KERN_ALERT "%s directory create failed.....\n", SWITCH_DIR);
		goto SWITCH_DIR_FAIL;
	}
	printk(KERN_ALERT "%s directory create successful.....\n", SWITCH_DIR);

	for(i = 0; i < NUM_SUPPORTED_BUTTON_NUM; i++)
	{
		switch_entry = create_proc_read_entry(button_info[i].name, 0, switch_dir, read_button_status, NULL);
		if(switch_entry == NULL)
		{
			printk(KERN_ALERT "%s/%s create failed.....\n", SWITCH_DIR, button_info[i].name);
			goto SWITCH_FILE_FAIL;
		}
		printk(KERN_ALERT "%s/%s create successful.....\n", SWITCH_DIR, button_info[i].name);
		switch_entry->data = (void *)&button_info[i];
	}
#endif

	return 0;

#if defined(CONFIG_BUFFALO_PLATFORM)
SWITCH_FILE_FAIL:
	for (i = 0; i < NUM_SUPPORTED_BUTTON_NUM; i++)
	{
		remove_proc_entry(button_info[i].name, switch_dir);
	}

SWITCH_DIR_FAIL:
	remove_proc_entry(SWITCH_DIR, NULL);
MODULE_DIR_FAIL:
	return -1;
#endif
}

/*+--------------------------------------------+
Cleanup procedure, unregister IRQ3 and restore 
system BIOS IT8721 GPIO34/35 settings.

Returns:
+--------------------------------------------+*/

static void __exit matsubtn_exit_cleanup(void)
{
	disable_irq(3);
	free_irq(3, matsu_interrupt);
	//Restore GPIO34/35 settings
	EnterMBPnP();

	outb(IT87_LDN_ADDR, CONFIG_ADDR_PORT);
	outb(IT87_GPIO_LDN, CONFIG_DATA_PORT);
	
	outb(IT87_REG_PNL_DBOUNCE0, CONFIG_ADDR_PORT);
	outb(0x00, CONFIG_DATA_PORT);
	outb(IT87_REG_PNL_DBOUNCE1, CONFIG_ADDR_PORT);
	outb(0x00, CONFIG_DATA_PORT);
	
	outb(IT87_REG_PNL_IRQ_SEL, CONFIG_ADDR_PORT);
	outb(0x00, CONFIG_DATA_PORT);
	
	ExitMBPnP();

	printk(KERN_INFO "MODULE %s %s HAS BEEN REMOVED!\n", DRVNAME, DRVVER);
}

module_init(matsubtn_init);
module_exit(matsubtn_exit_cleanup);
MODULE_LICENSE("GPL");
