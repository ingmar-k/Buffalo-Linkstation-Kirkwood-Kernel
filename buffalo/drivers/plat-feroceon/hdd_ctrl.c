/*
 * HDD control driver - 0.1
 * 1. Detect if HDD is presented or not.
 *
 *
 */
#include <linux/module.h>

#include "boardEnv/mvBoardEnvLib.h"
#include "boardEnv/mvBoardEnvSpec.h"
#include "BuffaloSataHotplug.h"
#include "BuffaloGpio.h"
#include "buffalo/kernevnt.h"
#include "gpp/mvGpp.h"
#include "gpp/mvGppRegs.h"

#define AUTHOR			"(C) BUFFALO INC."
#define DRIVER_NAME		"Buffalo GPIO SATA Hotplug Event Driver"
#define BUFFALO_DRIVER_VER	"1.00"

//#define DEBUG_SATA_HOTPLUG
#ifdef DEBUG_SATA_HOTPLUG
	#define TRACE(x)	x
#else
	#define TRACE(x)
#endif

static MV_U32 initial_polarity_val;
static MV_U32 initial_polarity_val_high;

static int
initSataHotplug(void)
{
	initial_polarity_val = MV_REG_READ(GPP_DATA_IN_POL_REG(0));
	initial_polarity_val_high = MV_REG_READ(GPP_DATA_IN_POL_REG(1));

	printk("initial_polarity_val = 0x%08x\n", initial_polarity_val);
	printk("initial_polarity_val_high = 0x%08x\n", initial_polarity_val_high);

	return 0;
}

static int
getSataHotPlugDevId(const int index, void** dev_id)
{
	int pinNum = mvBoardGpioPinNumGet(BOARD_GPP_SATA_HOT, index);

	if (pinNum == MV_ERROR) {
		return -1;
	}

	if (!bfGppInRegBitTest(pinNum)) {
		bfGppPolarityRegBitInv(pinNum);
	}

	*dev_id = (void*)pinNum;
	return 0;
}

static SATA_PLUG_STATE
getSataHotplugStat(void* dev_id)
{
	int pinNum = (int)dev_id;
	int pinstat = bfGppInRegBitTest(pinNum);
	SATA_PLUG_STATE plugstat;

	if (pinstat & bfGppPolarityRegBitTest(pinNum))
		plugstat = SATA_STAT_PLUGGED;
	else if (!pinstat & bfGppPolarityRegBitTest(pinNum))
		plugstat = SATA_STAT_UNPLUGGED;
	else if (pinstat & !bfGppPolarityRegBitTest(pinNum))
		plugstat = SATA_STAT_UNPLUGGED;
	else
		plugstat = SATA_STAT_PLUGGED;

	return plugstat;
}

static int
resetSataHotplugDev(struct sata_hotplug_data *data)
{
	int pinNum = (int)data->dev_id;
	if (data->lastplugstat == SATA_STAT_UNPLUGGED) {
		if(pinNum < 32)
			mvGppPolaritySet(0, BIT(pinNum), initial_polarity_val);
		else
			mvGppPolaritySet(1, BIT(pinNum - 32), initial_polarity_val_high);
	}
	else {
		if(pinNum < 32)
			mvGppPolaritySet(0, BIT(pinNum), ~initial_polarity_val);
		else
			mvGppPolaritySet(1, BIT(pinNum - 32), ~initial_polarity_val_high);
	}

	return 0;
}

static int
getSataHotplugIrq(void* dev_id)
{
	int pinNum = (int)dev_id;

	return IRQ_GPP_START + pinNum;
}

static void
resetSataHotplugIrq(void* dev_id)
{
	int pinNum = (int)dev_id;

	if(pinNum < 32)
		bfRegSet(GPP_INT_CAUSE_REG(0), BIT(pinNum), 0x0);
	else
		bfRegSet(GPP_INT_CAUSE_REG(1), BIT(pinNum - 32), 0x0);
}

struct sata_hotplug_ops hops = {
	.init = initSataHotplug,
	.getdev = getSataHotPlugDevId,
	.getstat = getSataHotplugStat,
	.resetdev = resetSataHotplugDev,
	.getirq = getSataHotplugIrq,
	.resetirq = resetSataHotplugIrq,
};

static int __init hdd_ctrl_init(void)
{
	BuffaloSataHotplug_init(&hops);

	printk("%s %s Ver.%s installed.\n", DRIVER_NAME, AUTHOR, BUFFALO_DRIVER_VER);
	return 0;
}

static void __exit hdd_ctrl_exit(void)
{
	printk("%s %s uninstalled.\n", DRIVER_NAME, BUFFALO_DRIVER_VER);
}

module_init(hdd_ctrl_init);
module_exit(hdd_ctrl_exit);
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DRIVER_NAME);
MODULE_VERSION(BUFFALO_DRIVER_VER);
MODULE_LICENSE("GPL");
