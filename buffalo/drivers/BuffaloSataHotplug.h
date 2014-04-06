#if !defined(_BUFFALO_SATA_HOTPLUG_H_)
#define _BUFFALO_SATA_HOTPLUG_H_

#include <linux/timer.h>

typedef enum _sata_plug_state {
	SATA_STAT_UNKNOWN,
	SATA_STAT_PLUGGED,
	SATA_STAT_UNPLUGGED,
}SATA_PLUG_STATE;

typedef int (init_t)(void);
typedef int (get_dev_id_t)(const int index, void** dev_id);
typedef int (get_stat_t)(void *dev_id);
typedef int (reset_dev_t)(struct sata_hotplug_data *data);
typedef int (get_irq_num_t)(void* dev_id);
typedef int (reset_irq_t)(void* dev_id);

struct sata_hotplug_ops {
	init_t		*init;
	get_dev_id_t	*getdev;
	get_stat_t	*getstat;
	reset_dev_t	*resetdev;

	get_irq_num_t	*getirq;
	reset_irq_t	*resetirq;
};

struct sata_hotplug_data {
	int index;
	    /* index in device */
	void *dev_id;
	    /* device identifier(pinNum, data-structure, etc...) */
	SATA_PLUG_STATE lastplugstat;
	    /* last data of device */

	struct timer_list timer;
	unsigned int loops;

	struct sata_hotplug_ops	*hops;
};

int BuffaloSataHotplug_init(struct sata_hotplug_ops *hops);
#endif
