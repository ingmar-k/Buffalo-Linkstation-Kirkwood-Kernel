#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/timer.h>
#include <linux/irq.h>

#include "buffalo/kernevnt.h"
#include "BuffaloSataHotplug.h"
#include "buffalo_proc_entry.h"

#define PLUGGED_EVENT_MSG	"SATA %d plugged"
#define UNPLUGGED_EVENT_MSG	"SATA %d unplugged"

// for polling timer
#define SATA_POL_INTERVAL	HZ/100
#define SATA_POL_LOOPS		10

int satahotplugdebug = 0;
core_param(satahotplugdebug, satahotplugdebug, int, 0644);

static int
BuffaloSataHotplugReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len = 0;
	struct sata_hotplug_data *sdata = (struct sata_hotplug_data *)data;

	if (sdata->lastplugstat == SATA_STAT_PLUGGED || satahotplugdebug)
		len += sprintf(page + len, "plugged\n");
	else if (sdata->lastplugstat == SATA_STAT_UNPLUGGED)
		len += sprintf(page + len, "unplugged\n");
	else
		len += sprintf(page + len, "unknown\n");

	if (off + count >= len)
		*eof = 1;

	if (len < off)
		return 0;

	*start = page + off;

	return ((count < len - off) ? count : len - off);
}

static void PollingSATAHotplug(unsigned long arg)
{
	struct sata_hotplug_data *data = (struct sata_hotplug_data *)arg;
	SATA_PLUG_STATE plugstat = data->hops->getstat(data->dev_id);

	char buf[32];

	if (data->lastplugstat == plugstat)
		data->loops = SATA_POL_LOOPS;
	else
		--data->loops;

	if (data->loops == 0) {
		data->lastplugstat = plugstat;

		if (plugstat == SATA_STAT_PLUGGED)
			sprintf(buf, PLUGGED_EVENT_MSG, data->index);
		else
			sprintf(buf, UNPLUGGED_EVENT_MSG, data->index);

		if (data->hops->resetdev)
			data->hops->resetdev(data);

		buffalo_kernevnt_queuein(buf);
	}

	if (data->loops || !data->hops->getirq) {
		data->timer.expires += SATA_POL_INTERVAL;
		add_timer(&data->timer);
	} else {
		enable_irq(data->hops->getirq(data->dev_id));
	}
}

static void
registerSataHotplugPolling(struct sata_hotplug_data *data)
{
	init_timer(&data->timer);
	data->timer.expires = jiffies + SATA_POL_INTERVAL;
	data->timer.function = PollingSATAHotplug;
	data->timer.data = (unsigned long)data;
	add_timer(&data->timer);
}

static irqreturn_t
SataHotplugInterrupts(int irq, void *dev_id)
{
	struct sata_hotplug_data *data = (struct sata_hotplug_data *)dev_id;

	if (data->hops->resetirq)
		data->hops->resetirq(data->dev_id);

	disable_irq_nosync(irq);

	registerSataHotplugPolling(data);

	return IRQ_HANDLED;
}

int BuffaloSataHotplug_init(struct sata_hotplug_ops *hops)
{
	int i;
	int retval;
	struct sata_hotplug_data *data;
	struct proc_entry_data pedata;
	void* dev_id;

	if (hops->init && (retval = hops->init()) != 0)
		return retval;

	for (i = 0; hops->getdev(i, &dev_id) != -1; i++) {

		data = kmalloc(sizeof(struct sata_hotplug_data), 0);
		if (!data) {
			printk("%s> kmalloc() failed. SATA%d Hotplug Interrupt disabled.\n",
			       __FUNCTION__, i);
			continue;
		}

		data->index = i;
		data->dev_id = dev_id;
		data->lastplugstat = SATA_STAT_UNKNOWN;
		data->loops = SATA_POL_LOOPS;

		data->hops = hops;

		if (hops->getirq) {
			retval = request_irq(hops->getirq(data->dev_id),
					     SataHotplugInterrupts,
					     0,
					     "SataHotplug",
					     (void*)data);

			if (retval) {
				printk("%s> request_irq() failed. SATA%d Hotplug Interrupt disabled.\n",
				       __FUNCTION__, i);
				kfree(data);
			}
		} else {
			registerSataHotplugPolling(data);
		}

		sprintf(pedata.path, "gpio/hotplug/sata%d", i);
		pedata.mode = 0;
		pedata.readproc = BuffaloSataHotplugReadProc;
		pedata.writeproc = NULL;
		pedata.data = data;
		make_proc_entry(&pedata);
	}

	return 0;
}

