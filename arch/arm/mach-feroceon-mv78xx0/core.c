/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
/* #include <linux/sysdev.h> #if LINUX_VERSION_CODE < KERNEL_VERSION(3,3,4) */
#include <asm/mach/time.h>
#include <linux/clocksource.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <mach/system.h>

#include <linux/tty.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/serial_reg.h>
#include <asm/serial.h>
#include <plat/cache-feroceon-l2.h>

#include <mach/serial.h>
#include <asm/string.h>

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cpu/mvCpu.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "mvDebug.h"
#include "mvSysHwConfig.h"
#include "pex/mvPexRegs.h"
#include "cntmr/mvCntmr.h"
#include "gpp/mvGpp.h"

#if defined(CONFIG_MV_INCLUDE_CESA)
#include "cesa/mvCesa.h"
#endif

extern unsigned int irq_int_type[];

/* for debug putstr */
#include <mach/uncompress.h> 

static char arr[256];

#ifdef MV_INCLUDE_EARLY_PRINTK
void mv_early_printk(char *fmt,...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf(arr,fmt,args);
	va_end(args);
	putstr(arr);
}
#endif


extern void __init mv_map_io(void);
extern void __init mv_init_irq(void);
extern struct sys_timer mv_timer;
extern MV_CPU_DEC_WIN* mv_sys_map(void);
#if defined(CONFIG_MV_INCLUDE_CESA)
extern u32 mv_crypto_base_get(void);
#endif
unsigned int support_wait_for_interrupt = 0x1;

u32 mvTclk = 166666667;
u32 mvSysclk = 200000000;
u32 mvIsUsbHost = 1;


u8	mvMacAddr[CONFIG_MV_ETH_PORTS_NUM][6];
u16	mvMtu[CONFIG_MV_ETH_PORTS_NUM] = {0};
extern MV_U32 gBoardId; 
extern unsigned int elf_hwcap;

#if defined(CONFIG_BUFFALO_UBOOT_PARAMS) // BUFFALO_PLATFORM
extern u32 env_addr;
extern u32 env_size;
extern u32 env_offset;
#endif // CONFIG_BUFFALO_UBOOT_PARAMS
 
static int __init parse_tag_mv_uboot(const struct tag *tag)
{
    	unsigned int mvUbootVer = 0;
	int i = 0;
 
	mvUbootVer = tag->u.mv_uboot.uboot_version;
	mvIsUsbHost = tag->u.mv_uboot.isUsbHost;

        printk("Using UBoot passing parameters structure\n");
  
	gBoardId =  (mvUbootVer & 0xff);
	for (i = 0; i < CONFIG_MV_ETH_PORTS_NUM; i++) {
#if defined (CONFIG_MV78XX0_Z0) || defined (CONFIG_OVERRIDE_ETH_CMDLINE)
		memset(&mvMacAddr[i][0], 0, 6);
		mvMtu[i] = 0;
#else			
		memcpy(&mvMacAddr[i][0], &tag->u.mv_uboot.macAddr[i][0], 6);
		mvMtu[i] = tag->u.mv_uboot.mtu[i];
#endif    	
	}

#if defined(CONFIG_BUFFALO_UBOOT_PARAMS) // BUFFALO_PLATFORM
	{
		extern u32 env_addr;
		extern u32 env_size;
		extern u32 env_offset;
		extern char umsg[UMSG_SIZE];

#if 1
		// for yabuta's fuckin u-boot.
		if (tag->u.mv_uboot.env_addr == 0x00000000)
		{
			printk("u.mv_uboot.env_addr is 0x0! Using fixed value.\n");
			env_addr = 0xfff80000;
			env_size = 0x00001000;
			env_offset = 0x00000000;
		} else {
			printk("u.mv_uboot.env_addr is 0x%08x. Using tag's value.\n", tag->u.mv_uboot.env_addr);
			env_addr = tag->u.mv_uboot.env_addr;
			env_size = tag->u.mv_uboot.env_size;
			env_offset = tag->u.mv_uboot.env_offset;
		}
#else
		env_addr = tag->u.mv_uboot.env_addr;
		env_size = tag->u.mv_uboot.env_size;
		env_offset = tag->u.mv_uboot.env_offset;
#endif
		memcpy(umsg, tag->u.mv_uboot.umsg, sizeof(umsg));
	}
#endif // CONFIG_BUFFALO_UBOOT_PARAMS

	return 0;
}
                                                                                                                             
__tagtable(ATAG_MV_UBOOT, parse_tag_mv_uboot);

#ifdef CONFIG_MV_INCLUDE_CESA
unsigned char*  mv_sram_usage_get(int* sram_size_ptr)
{
    int used_size = 0;

#if defined(CONFIG_MV_CESA)
    used_size = sizeof(MV_CESA_SRAM_MAP);
#endif

    if(sram_size_ptr != NULL)
        *sram_size_ptr = _8K - used_size;

    return (char *)(mv_crypto_base_get() + used_size);
}
#endif


void print_board_info(void)
{
    char name_buff[50];
    printk("\n  Marvell Development Board (LSP Version %s)",LSP_VERSION);

    mvBoardNameGet(name_buff);
    printk("-- %s ",name_buff);

    mvCtrlModelRevNameGet(name_buff);
    printk(" Soc: %s",  name_buff);
#if defined(MV_CPU_LE)
	printk(" LE");
#else
	printk(" BE");
#endif
    printk("\n\n");
    printk(" Detected Tclk %d and SysClk %d \n",mvTclk, mvSysclk);
}


/*****************************************************************************
 * UART
 ****************************************************************************/
static struct resource mv_uart_resources[] = {
	{
		.start		= PORT_BASE(0),
		.end		= PORT_BASE(0) + 0xff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start          = IRQ_UART(0),
		.end            = IRQ_UART(0),
		.flags          = IORESOURCE_IRQ,
	},
	{
		.start		= PORT_BASE(1),
		.end		= PORT_BASE(1) + 0xff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start          = IRQ_UART(1),
		.end            = IRQ_UART(1),
		.flags          = IORESOURCE_IRQ,
	},
};

static struct plat_serial8250_port mv_uart_data[] = {
	{
		.mapbase	= PORT_BASE(0),
		.membase	= (char *)PORT_BASE(0),
		.irq		= IRQ_UART(0),
		.flags		= UPF_SKIP_TEST | UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
	},
	{
		.mapbase	= PORT_BASE(1),
		.membase	= (char *)PORT_BASE(1),
		.irq		= IRQ_UART(1),
		.flags		= UPF_SKIP_TEST | UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
	},
	{ },
};

static struct platform_device mv_uart = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM,
	.dev			= {
		.platform_data	= mv_uart_data,
	},
/* warning: support 2 UART ports, initialize only 1 */
	.num_resources		= 2, /*ARRAY_SIZE(mv_uart_resources),*/
	.resource		= mv_uart_resources,
};

#ifdef CONFIG_BUFFALO_PLATFORM
static struct resource mv_uart2_resources[] = {
	{
		.start		= PORT_BASE(2),
		.end		= PORT_BASE(2) + 0xff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= IRQ_UART(2),
		.end		= IRQ_UART(2),
		.flags		= IORESOURCE_IRQ,
	},
};
static struct plat_serial8250_port mv_uart2_data[] = {
	{
		.mapbase	= PORT_BASE(2),
		.membase	= (char *)PORT_BASE(2),
		.irq		= IRQ_UART(2),
		.flags		= UPF_SKIP_TEST | UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_MEM,
		.regshift	= 2,
	},
	{ },
};
static struct platform_device mv_uart2 = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM2,
	.dev			= {
		.platform_data	= mv_uart2_data,
	},
	.num_resources		= ARRAY_SIZE(mv_uart2_resources),
	.resource		= mv_uart2_resources,
};
#endif

static void serial_initialize(void)
{
#if defined(CONFIG_MV78200) || defined(CONFIG_MV632X)
	/*One UART per CPU*/	
	if (MV_TRUE == mvSocUnitIsMappedToThisCpu(UART0))
	{		
		mv_uart.resource = &mv_uart_resources[0]; 
		mv_uart.dev.platform_data = &mv_uart_data[0];
		mv_uart_data[1].flags = 0;
		mv_uart_data[0].uartclk = mvTclk;
		platform_device_register(&mv_uart);
	}
	else if (MV_TRUE == mvSocUnitIsMappedToThisCpu(UART1))
	{	
		mv_uart.resource = &mv_uart_resources[2]; 
		mv_uart.dev.platform_data = &mv_uart_data[1];
		mv_uart_data[1].uartclk = mvTclk;
		platform_device_register(&mv_uart);
	}	
#else
	mv_uart_data[0].uartclk = mv_uart_data[1].uartclk = mvTclk;
	platform_device_register(&mv_uart);
#ifdef CONFIG_BUFFALO_PLATFORM
	mv_uart2_data[0].uartclk = mvTclk;
	platform_device_register(&mv_uart2);
#endif
#endif
}

static void __init mv_vfp_init(void)
{

#if defined CONFIG_VFP_FASTVFP
	printk("VFP initialized to Run Fast Mode.\n");
#endif
}


#ifdef CONFIG_MV_ETHERNET
/*****************************************************************************
 * Ethernet
 ****************************************************************************/
static struct platform_device mv88fx_eth = {
	.name		= "mv88fx_eth",
	.id		= 0,
	.num_resources	= 0,
};
#endif

static void __init dd_l2_init(void)
{
#ifdef CONFIG_CACHE_FEROCEON_L2_WRITETHROUGH
	MV_REG_BIT_SET(CPU_CTRL_STAT_REG(whoAmI()), 0x10);
	feroceon_l2_init(1);
#else
	MV_REG_BIT_RESET(CPU_CTRL_STAT_REG(whoAmI()), 0x10);
	feroceon_l2_init(0);
#endif
}


static void __init mv_init(void)
{
#if defined(CONFIG_MV78200) || defined(CONFIG_MV632X)
	MV_U32 pexClkOff[2];
#endif
#ifdef CONFIG_CACHE_FEROCEON_L2
	dd_l2_init();
#endif

#if defined(CONFIG_MV78200) || defined(CONFIG_MV632X)
	mvSemaLock(MV_SEMA_PEX0);
	pexClkOff[0] = MV_REG_READ(0x2011C) & PMC_PEX_UP(0);
	pexClkOff[1] = MV_REG_READ(0x2411C) & PMC_PEX_UP(0);
	if (!(pexClkOff[0] && pexClkOff[1]))
	{
		MV_REG_BIT_SET(POWER_MNG_CTRL_REG0, PMC_PEX_MASK(0));
		MV_REG_BIT_SET(POWER_MNG_CTRL_REG1, PMC_PEX_MASK(0));
	}
#endif
	/* init the Board environment */
	mvBoardEnvInit();

        /* init the controller environment */
        if (mvCtrlEnvInit() ) {
            printk( "Controller env initialization failed.\n" );
            return;
        }


	/* Init the CPU windows setting and the access protection windows. */
	if (mvCpuIfInit(mv_sys_map())) {

		printk( "Cpu Interface initialization failed.\n" );
		return;
	}
	/*NAND flash decoding window for AMC board */
	if (mvBoardIdGet() == RD_78XX0_AMC_ID)
	{		
		MV_AHB_TO_MBUS_DEC_WIN addrDecWin;
		addrDecWin.target = DEV_BOOCS;
		addrDecWin.addrWin.baseLow = DEVICE_CS2_BASE;
		addrDecWin.addrWin.baseHigh = 0;
		addrDecWin.addrWin.size = _1M;
		addrDecWin.enable = MV_TRUE;
		mvAhbToMbusWinSet(whoAmI(), 12, &addrDecWin);
	}

#if defined (CONFIG_MV78XX0_Z0)
	mvCpuIfBridgeReorderWAInit();
#endif

    	/* Init Tclk & SysClk */
    	mvTclk = mvBoardTclkGet();
   	mvSysclk = mvBoardSysClkGet();
	
        support_wait_for_interrupt = 1;
  
#ifdef CONFIG_JTAG_DEBUG
            support_wait_for_interrupt = 0; /*  for Lauterbach */
#endif
	mv_vfp_init();	
	elf_hwcap &= ~HWCAP_JAVA;

   	serial_initialize();

	/* At this point, the CPU windows are configured according to default definitions in mvSysHwConfig.h */
	/* and cpuAddrWinMap table in mvCpuIf.c. Now it's time to change defaults for each platform.         */
	mvCpuIfAddrDecShow(whoAmI());

    	print_board_info();
#if defined(CONFIG_MV78200) || defined(CONFIG_MV632X)
	if (!pexClkOff[0])	
		MV_REG_BIT_RESET(POWER_MNG_CTRL_REG0, PMC_PEX_MASK(0));
	if (!pexClkOff[1])
		MV_REG_BIT_RESET(POWER_MNG_CTRL_REG1, PMC_PEX_MASK(0));
	mvSemaUnlock(MV_SEMA_PEX0);
#endif

#ifdef CONFIG_MV_ETHERNET
    /* ethernet */
	platform_device_register(&mv88fx_eth);
#endif

}

#if defined(CONFIG_MV78200) || defined(CONFIG_MV632X)


static void __init early_resmap_cpu0(char **p)
{
	char* tmp = strchr(*p, ' ');
	if (tmp) *tmp = '\0';
	if (MV_FALSE == mvSocUnitMapFillTable(*p, 0, strstr))		
		printk(KERN_ERR "Invalid command line for CPU0\n");
	if (tmp) *tmp = ' ';
}

static void __init early_resmap_cpu1(char **p)
{
	char* tmp = strchr(*p, ' ');
	if (tmp) *tmp = '\0';
	if (MV_FALSE == mvSocUnitMapFillTable(*p, 1, strstr))
		printk(KERN_ERR "Invalid command line for CPU1\n");
	if (tmp) *tmp = ' ';
}

__early_param("cpu0=", early_resmap_cpu0);
__early_param("cpu1=", early_resmap_cpu1);

#endif


MACHINE_START(FEROCEON_MV78XX0, "Feroceon-MV78XX0")
    /* MAINTAINER("MARVELL") */
#if 0 /* LINUX_VERSION_CODE < KERNEL_VERSION(3,3,4) */
    .phys_io = 0xf1000000,
    .io_pg_offst = ((0xf1000000) >> 18) & 0xfffc,
    .boot_params = PHYS_OFFSET+0x00000100,
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,3,4) */
    .map_io = mv_map_io,
    .init_irq = mv_init_irq,
    .timer = &mv_timer,
    .init_machine = mv_init,
MACHINE_END

