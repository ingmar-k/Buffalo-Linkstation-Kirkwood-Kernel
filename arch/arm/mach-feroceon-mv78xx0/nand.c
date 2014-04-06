
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <asm/io.h>
#include "ctrlEnv/sys/mvCpuIf.h"


static struct mtd_info *mv_mtd;
static unsigned long baseaddr;

#ifdef CONFIG_MTD_PARTITIONS
#if defined(CONFIG_BUFFALO_PLATFORM)
  #define MV_NUM_OF_NAND_PARTS 2
  static struct mtd_partition parts_info[] = {
	{ .name = "nboot",
	  .offset = 0,
	  .size = 32 * 1024 * 1024 },
	{ .name = "nroot",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = MTDPART_SIZ_FULL },
  };

#else
  #define MV_NUM_OF_NAND_PARTS 3
  static struct mtd_partition parts_info[] = {
	{ .name = "u-boot",
	  .offset = 0,
	  .size = 1 * 1024 * 1024 },
	{ .name = "uImage",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = 2 * 1024 * 1024 },
	{ .name = "root",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = MTDPART_SIZ_FULL },
  };
#endif // of CONFIG_BUFFALO_PLATFORM
static const char *part_probes[] __initdata = { "cmdlinepart", NULL };
#endif

static void board_hwcontrol(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	struct nand_chip *this = (struct nand_chip *)mtd->priv;
	if (ctrl & NAND_CTRL_CHANGE) {        
		this->IO_ADDR_W = (void __iomem *)((unsigned long)this->IO_ADDR_W & ~3);
		ctrl &= ~NAND_CTRL_CHANGE;
		switch(ctrl) {
			case NAND_CTRL_CLE: 
				this->IO_ADDR_W = (void __iomem *)((unsigned long)this->IO_ADDR_W | 1); //x8=>1, x16=>2
				break;
			case NAND_CTRL_ALE:
				this->IO_ADDR_W = (void __iomem *)((unsigned long)this->IO_ADDR_W | 2); //x8=>2, x16=>4
				break;
		}				
	}
	if (cmd != NAND_CMD_NONE) 
	{
		writeb(cmd, this->IO_ADDR_W);		
	}	
}

static void mv_nand_reset(struct nand_chip *this)
{
	/*reset*/
	writeb(0xff, (void __iomem *)((unsigned long)this->IO_ADDR_W | 1));
}

int __init mv_nand_init(void)
{
	struct nand_chip *this;
	int err = 0;
	int num_of_parts = 0;
	const char *part_type = 0;
	struct mtd_partition *mtd_parts = 0;
	u32 physaddr;
	int nand_dev;
	MV_CPU_DEC_WIN addr_win;
#if defined(CONFIG_MV78200) || defined(CONFIG_MV632X)
	if (MV_FALSE == mvSocUnitIsMappedToThisCpu(NAND_FLASH))
	{	
		printk(KERN_INFO"NAND flash is not mapped to this CPU\n");
		return -ENODEV;
	}
#endif
	if (mvBoardIdGet() != RD_78XX0_AMC_ID)
	{
		nand_dev = mvBoardGetDevCSNum(0, BOARD_DEV_NAND_FLASH);
		if (MAX_TARGETS == nand_dev) {
			printk("NAND init: NAND device not found on board\n");
			err = -ENODEV;
			goto out;
		}
		if( MV_OK != mvCpuIfTargetWinGet(nand_dev, &addr_win) ) {
			printk("Failed to init NAND MTD (window %d err).\n", nand_dev);
			err = -ENODEV;
			goto out;
		}

		if(!addr_win.enable) {
			printk("Failed to init NAND MTD (boot-CS window disabled).\n" );
			err = -ENODEV;
			goto out;
		}
		physaddr = addr_win.addrWin.baseLow;
	}
	else
	{	/*NAND address for AMC board*/
		physaddr = DEVICE_CS2_BASE;
	}	
		
	mv_mtd = (struct mtd_info *)kmalloc(sizeof(struct mtd_info)+sizeof(struct nand_chip), GFP_KERNEL);
	if(!mv_mtd){
		printk("Failed to allocate NAND MTD structure\n");
		err = -ENOMEM;
		goto out;
	}

	memset((char*)mv_mtd,0,sizeof(struct mtd_info)+sizeof(struct nand_chip));

	baseaddr = (unsigned long)ioremap(physaddr, 1024);
	if(!baseaddr) {
		printk("Failed to remap NAND MTD\n");
		err = -EIO;
		goto out_mtd;
	}	
	this = (struct nand_chip *)((char *)mv_mtd+sizeof(struct mtd_info));
	mv_mtd->priv = this;
	this->IO_ADDR_R = this->IO_ADDR_W = (void __iomem *)baseaddr;
	this->cmd_ctrl = board_hwcontrol;
	this->ecc.mode = NAND_ECC_SOFT;
	this->chip_delay = 25;
	mv_nand_reset(this);
	if(nand_scan(mv_mtd,1)) {
		err = -ENXIO;
		goto out_ior;
	}

#ifdef CONFIG_MTD_PARTITIONS
#if 1 /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,4) */
	mv_mtd->name = "nand_mtd";
	mtd_device_parse_register(mv_mtd, part_probes,
							  NULL,
							  mtd_parts,
							  MV_NUM_OF_NAND_PARTS);
#else
        mv_mtd->name = "nand_mtd";
        num_of_parts = parse_mtd_partitions(mv_mtd,part_probes,&mtd_parts,0);
        if(num_of_parts > 0)
                part_type = "command line";
        else
                num_of_parts = 0;
        if(num_of_parts == 0) {
                mtd_parts = parts_info;
                num_of_parts = MV_NUM_OF_NAND_PARTS;
                part_type = "static";
        }

	printk("Using %s partition definition\n", part_type);
	add_mtd_partitions(mv_mtd, mtd_parts, num_of_parts);
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,4) */
#endif
	goto out;

out_ior:
	iounmap((void *)baseaddr);
out_mtd:
	kfree(mv_mtd);
out:
	return err;
}

module_init(mv_nand_init);

#ifdef MODULE
static void __exit board_cleanup(void)
{
	nand_release(mv_mtd);
	iounmap((void*)baseaddr);
	kfree(mv_mtd);
}
module_exit(board_cleanup);
#endif

