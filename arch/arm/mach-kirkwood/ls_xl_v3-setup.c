/*
 * arch/arm/mach-kirkwood/ls_xl_v3-setup.c
 *
 * Buffalo Linkstation Live V3 (LS_XL8C) Board Setup
 * Buffalo NAS-1bay_LS_XL_M6192 V3.0
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 * 
 * Based on the setup files of a number of different Kirkwood files.
 * Edited by Ingmar Klein <ingmar.klein@hs-augsburg.de>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/ata_platform.h>
#include <linux/mtd/partitions.h>
#include <linux/mv643xx_eth.h>
#include <linux/gpio.h>
#include <linux/spi/flash.h>
#include <linux/spi/spi.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <mach/kirkwood.h>
#include <plat/orion-gpio.h>
#include "common.h"
#include <linux/mtd/physmap.h>
#include <linux/leds.h>
#include "mpp.h"
#include "ls_xl_v3-setup.h"


/*****************************************************************************
 * 512KB SPI Flash on BOOT Device
 ****************************************************************************/
static struct mtd_partition ls_xl_v3_spi_partitions[] = {
	{
		.name		= "uboot",
		.size		= 0x60000,
		.offset		= 0x00000,
		.mask_flags	= MTD_WRITEABLE,
	},
	{
		.name		= "dtb",
		.size		= 0x10000,
		.offset		= 0x60000,
		.mask_flags	= MTD_WRITEABLE,
	},
	{
		.name		= "uboot_env",
		.size		= 0x10000,
		.offset		= 0x70000,
	}
};

static struct flash_platform_data ls_xl_v3_spi_slave_data = {
	.type		= "m25p40",
	.parts		= ls_xl_v3_spi_partitions,
	.nr_parts	= ARRAY_SIZE(ls_xl_v3_spi_partitions),
};

static struct spi_board_info __initdata ls_xl_v3_spi_slave_info[] = {
	{
		.modalias	= "m25p80",
		.platform_data	= &ls_xl_v3_spi_slave_data,
		.irq		= -1,
		//.max_speed_hz	= 20000000,
		//.bus_num	= 0,
		//.chip_select	= 0,
	}
};


static struct mtd_partition ls_xl_v3_nand_parts[] = {
	{
		.name = "kernel",
		.offset = 0,
		.size = 0x00300000
	}, {
		.name = "rootfs",
		.offset = MTDPART_OFS_NXTBLK,
		.size = 0x06D00000
	}, {
		.name = "swap",
		.offset = MTDPART_OFS_NXTBLK,
		.size = 0x01000000
	},
};

static struct mv643xx_eth_platform_data ls_xl_v3_ge00_data = {
	.phy_addr	= MV643XX_ETH_PHY_ADDR(0),
};

static struct mv_sata_platform_data ls_xl_v3_sata_data = {
	.n_ports	= 1,
};

/*****************************************************************************
 * LEDs attached to GPIO
 ****************************************************************************/
#define LS_XL_V3_GPIO_PWR_LED 17

//#define LS_XL_V3_GPIO_ALARM_LED 26
//#define LS_XL_V3_GPIO_INFO_LED 27

static struct gpio_led ls_xl_v3_gpio_led_pins[] = {
	{
		.name			= "ls_xl_v3:blue:power",
		.gpio			= LS_XL_V3_GPIO_PWR_LED,
		.default_trigger = "heartbeat",
		.active_low		= 1,
	},
};

static struct gpio_led_platform_data ls_xl_v3_gpio_led_data = {
	.leds		= ls_xl_v3_gpio_led_pins,
	.num_leds	= ARRAY_SIZE(ls_xl_v3_gpio_led_pins),
};

static struct platform_device ls_xl_v3_gpio_leds = {
	.name	= "leds-gpio",
	.id	= -1,
	.dev	= {
		.platform_data	= &ls_xl_v3_gpio_led_data,
	}
};

/*****************************************************************************
 * GPIO Data
 ****************************************************************************/

/* MVLSXL-GE-V2 */
/*#define MVLSXL_GE_V2_MPP0_7			0x21111111
#define MVLSXL_GE_V2_MPP8_15			0x00003300
#define MVLSXL_GE_V2_MPP16_23			0x00501100
#define MVLSXL_GE_V2_MPP24_31			0x00000000
#define MVLSXL_GE_V2_MPP32_35			0x00000000*/

/***********************************************************************
*** Original values, like found in the Marvell 3.3.4 kernel sources ****
****  #define MVLSXL_GE_V2_MPP0_7			0x21111111 *****************
************************************************************************
**** ---> according to "mpp.h" :

*
*   #define MPP0_NF_IO2         0x1
*   #define MPP1_NF_IO3         0x1
*   #define MPP2_NF_IO4         0x1
*   #define MPP3_NF_IO5         0x1
*   #define MPP4_NF_IO6         0x1
*   #define MPP5_NF_IO7         0x1
*   #define MPP6_SYSRST_OUTn    0x1
*   #define MPP7_SPI_SCn        0x2
* 

************************************************************************ 
****  #define MVLSXL_GE_V2_MPP8_15			0x00003300  ****************
************************************************************************
**** ---> according to "mpp.h" :
*
*   #define MPP8_GPIO           0x0
*   #define MPP9_GPIO           0x0
*   #define MPP10_UART0_TXD     0x3
*   #define MPP11_UART0_RXD     0x3
*   #define MPP12_GPO     		0x0
*   #define MPP13_GPIO	        0x0
*   #define MPP14_GPIO   		0x0
*   #define MPP15_GPIO   		0x0
*

************************************************************************ 
****  #define MVLSXL_GE_V2_MPP16_23			0x00501100  ****************
************************************************************************
**** ---> according to "mpp.h" :
*
*   #define MPP16_GPIO          0x0
*   #define MPP17_GPIO          0x0
*   #define MPP18_NF_IO0        0x1
*   #define MPP19_NF_IO1        0x1
*   #define MPP20_GPIO          0x0
*   #define MPP21_SATA0_ACTn    0x5
*   #define MPP22_GPIO   		0x0
*   #define MPP23_GPIO   		0x0
*  
************************************************************************ 
****  #define MVLSXL_GE_V2_MPP24_31			0x00000000  ****************
****  #define MVLSXL_GE_V2_MPP32_35			0x00000000  ****************
************************************************************************
**** ---> according to "mpp.h" :
* 
*   #define MPP24_GPIO   		0x0
*   #define MPP25_GPIO   		0x0
*   #define MPP26_GPIO   		0x0
*   #define MPP27_GPIO   		0x0
*   #define MPP28_GPIO   		0x0
*   #define MPP29_GPIO   		0x0
*   #define MPP30_GPIO   		0x0
*   #define MPP31_GPIO   		0x0
*   #define MPP32_GPIO   		0x0
*   #define MPP33_GPO   		0x0
*   #define MPP34_GPIO   		0x0
*   #define MPP35_GPIO   		0x0
* 
************************************************************************ 
************************************************************************ 
***********************************************************************/


static unsigned int ls_xl_v3_mpp_config[] __initdata = {
	MPP0_NF_IO2, // nand(io2)
	MPP1_NF_IO3, // nand(io3)
	MPP2_NF_IO4, // nand(io4)
	MPP3_NF_IO5, // nand(io5)
	MPP4_NF_IO6, // nand(io6)
	MPP5_NF_IO7, // nand(io7)
	MPP6_SYSRST_OUTn, // sysrst(out)
	MPP10_UART0_TXD, // uart0(tx)
	MPP11_UART0_RXD, // uart0(rx)
	MPP14_GPIO, // gpio: hdd power control
	MPP15_UART1_TXD, // uart1(tx)
	MPP16_UART1_RXD, // uart1(rx)
	MPP17_GPIO, // gpio: power led (blue)
	MPP18_NF_IO0, // nand(io0)
	MPP19_NF_IO1, // nand(io1)
	0
};



/*****************************************************************************
 * Hard Disk power
 ****************************************************************************/

static int __initdata ls_xl_v3_gpio_hdd_power = 14;

void __init ls_xl_v3_hdd_power_init(void)
{
	int err;

	/* Power up the hard disk. */
	err = gpio_request(ls_xl_v3_gpio_hdd_power, NULL);
	if (err == 0) {
		err = gpio_direction_output(
				ls_xl_v3_gpio_hdd_power, 1);
			/* Free the HDD power GPIOs. This allow user-space to
			 * configure them via the gpiolib sysfs interface. */
			gpio_free(ls_xl_v3_gpio_hdd_power);
	}
	if (err)
		pr_err("Failed to power up HDD.\n");
}


/*****************************************************************************
 * GPIO HDD Power definition
 ****************************************************************************/
void __init ls_xl_v3_gpio_hdd_power_setup(void)
{
	int err;
	err = gpio_request(ls_xl_v3_gpio_hdd_power,"hdd_pwr_ctrl");
	if (err == 0)
	{
		err = gpio_direction_output(ls_xl_v3_gpio_hdd_power,1);
		if (err == 0)
		{
			err = gpio_export(ls_xl_v3_gpio_hdd_power,false);
			if (err == 0)
			{
				pr_info("Successfully exported GPIO 'hdd_pwr_ctrl' pin.\n");
			}
		}
		if (err)
			pr_err("Failed to set direction of GPIO 'hdd_pwr_ctrl'.\n");
	}
	if (err)
		pr_err("Failed to request GPIO 'hdd_pwr_ctrl' (GPIO # %d).\n",ls_xl_v3_gpio_hdd_power);
}


/*****************************************************************************
 * Board initialization
 ****************************************************************************/
 
static void __init ls_xl_v3_init(void)
{
	/*
	 * Basic setup. Needs to be called early.
	 */
	kirkwood_init(); //general init
	kirkwood_mpp_conf(ls_xl_v3_mpp_config); // config of the multi purpose pins (mpp)
	kirkwood_uart0_init(); //init uart0
	ls_xl_v3_hdd_power_init(); // powerup hdd
	ls_xl_v3_gpio_hdd_power_setup(); // setup hdd power control over gpio
	kirkwood_nand_init(ls_xl_v3_nand_parts, 
			   ARRAY_SIZE(ls_xl_v3_nand_parts),
			   25); //init NAND
	kirkwood_ge00_init(&ls_xl_v3_ge00_data); // init Gigabit Ethernet
	kirkwood_sata_init(&ls_xl_v3_sata_data); // init SATA
	platform_device_register(&ls_xl_v3_gpio_leds); //register LEDs
}

static int __init ls_xl_v3_pci_init(void)
{
	if (machine_is_ls_xl_v3())
		kirkwood_pcie_init(KW_PCIE0);

	return 0;
}
subsys_initcall(ls_xl_v3_pci_init);

MACHINE_START(LS_XL_V3, "Buffalo Linkstation Live V3 Board")
	/* Maintainer: NONE */
	.atag_offset	= 0x100,
	.init_machine	= ls_xl_v3_init,
	.map_io		= kirkwood_map_io,
	.init_early	= kirkwood_init_early,
	.init_irq	= kirkwood_init_irq,
	.init_time	= kirkwood_timer_init,
	.restart	= kirkwood_restart,
MACHINE_END
