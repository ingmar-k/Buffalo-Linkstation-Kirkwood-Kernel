#if defined(_HDD_CTRL_H__)
#else
#define _HDD_CTRL_H

/* Define Vendor and product id for ich9r */
#define INTEL_ICH9R_VENDOR_ID	0x8086
#define INTEL_ICH9R_DEVICE_ID	0x2916

#define GPIO_BAR_OFFSET		0x48
#define GPIO_USE_SEL		0x00
#define GP_IO_SEL		0x04
#define GP_LVL			0x0c
#define GP_LVL2			0x38
#define GPO_BLINK		0x18
#define GPI_INV			0x2c

/* Define GPIO
EVT board:
SATA Hot 0 --> GPIO2
SATA Hot 1 --> GPIO3
SATA Hot 2 --> GPIO4
SATA Hot 3 --> GPIO5
SATA Hot 4 --> GPIO15
SATA Hot 5 --> GPIO16
SATA Hot 6 --> GPIO18
SATA Hot 7 --> GPIO20

HDD_Power0 --> GPIO17
HDD_Power1 --> GPIO1
HDD_Power2 --> GPIO6
HDD_Power3 --> GPIO7
HDD_Power4 --> GPIO25
HDD_Power5 --> GPIO32
HDD_Power6 --> GPIO33
HDD_Power7 --> GPIO34

DVT board:
SATA Hot 0 --> GPIO2
SATA Hot 1 --> GPIO3
SATA Hot 2 --> GPIO4
SATA Hot 3 --> GPIO5
SATA Hot 4 --> GPIO15
SATA Hot 5 --> GPIO9
SATA Hot 6 --> GPIO13
SATA Hot 7 --> GPIO11

HDD_Power0 --> GPIO17
HDD_Power1 --> GPIO1
HDD_Power2 --> GPIO6
HDD_Power3 --> GPIO7
HDD_Power4 --> GPIO25
HDD_Power5 --> GPIO32
HDD_Power6 --> GPIO33
HDD_Power7 --> GPIO34
*/
/* for EVT and DVT (common) */
#define HDD0_PRESENT_BIT		0x00000004	/*GPIO2*/
#define HDD1_PRESENT_BIT		0x00000008	/*GPIO3*/
#define HDD2_PRESENT_BIT		0x00000010	/*GPIO4*/
#define HDD3_PRESENT_BIT		0x00000020	/*GPIO5*/
#define HDD4_PRESENT_BIT		0x00008000	/*GPIO15*/
#if defined(CONFIG_BUFFALO_MATSU_USE_EVT_BOARD)
/* for EVT */
  #define HDD5_PRESENT_BIT		0x00010000	/*GPIO16*/
  #define HDD6_PRESENT_BIT		0x00040000	/*GPIO18*/
  #define HDD7_PRESENT_BIT		0x00100000	/*GPIO20*/
#else
/* for DVT */
  #define HDD5_PRESENT_BIT		0x00000200	/*GPIO9*/
  #define HDD6_PRESENT_BIT		0x00002000	/*GPIO13*/
  #define HDD7_PRESENT_BIT		0x00000400	/*GPIO10, DVT2 board changed from GPI11 to GPI10*/
#endif

#define HDD0_POWER_BIT			0x00020000	/*GPIO17*/
#define HDD1_POWER_BIT			0x00000002	/*GPIO1*/
#define HDD2_POWER_BIT			0x00000040	/*GPIO6*/
#define HDD3_POWER_BIT			0x00000080	/*GPIO7*/
#define HDD4_POWER_BIT			0x02000000	/*GPIO25*/
#define HDD5_POWER_BIT			0x00000001	/*GPIO32*/
#define HDD6_POWER_BIT			0x00000002	/*GPIO33*/
#define HDD7_POWER_BIT			0x00000004	/*GPIO34*/

#define HDD0_PRESENT_MASK		0xfffffffb
#define HDD1_PRESENT_MASK		0xfffffff7
#define HDD2_PRESENT_MASK		0xffffffef
#define HDD3_PRESENT_MASK		0xffffffdf
#define HDD4_PRESENT_MASK		0xffff7fff
#if defined(CONFIG_BUFFALO_MATSU_USE_EVT_BOARD)
/* for EVT */
  #define HDD5_PRESENT_MASK		0xfffeffff
  #define HDD6_PRESENT_MASK		0xfffbffff
  #define HDD7_PRESENT_MASK		0xffefffff
#else
/* for DVT */
  #define HDD5_PRESENT_MASK		0xfffffdff
  #define HDD6_PRESENT_MASK		0xffffdfff
  #define HDD7_PRESENT_MASK		0xfffff7ff
#endif

#define HDD0_POWER_MASK			0xfffdffff
#define HDD1_POWER_MASK			0xfffffffd
#define HDD2_POWER_MASK			0xffffffbf
#define HDD3_POWER_MASK			0xffffff7f
#define HDD4_POWER_MASK			0xfdffffff
#define HDD5_POWER_MASK			0xfffffffe
#define HDD6_POWER_MASK			0xfffffffd
#define HDD7_POWER_MASK			0xfffffffb

/* Define for procfs directory and file */
#define MODULE_DIR			"buffalo/gpio"
#define HDD_PRESENT_DIR			"buffalo/gpio/hotplug"
#define HDD_POWER_DIR			"buffalo/gpio/power_control"

#define MAX_SUPPORTED_DISKS	8

///////////////// for hotplug /////////////
typedef enum _sata_plug_state {
	SATA_STAT_UNKNOWN,
	SATA_STAT_PLUGGED,
	SATA_STAT_UNPLUGGED,
}SATA_PLUG_STATE;

struct sata_hotplug_data_st {
	SATA_PLUG_STATE presentpinstat;
	SATA_PLUG_STATE prevplugstat;
	unsigned int loops;
};


///////////////// for hotplug /////////////
#endif  /* _HDD_CTRL_H */

