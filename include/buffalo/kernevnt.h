#ifndef _KERNEVNT_H_
#define _KERNEVNT_H_

// variable definition.
#define POWER_STATUS_REBOOTING                  1
#define POWER_STATUS_REBOOTING_UBOOT_PASSED     2
#define POWER_STATUS_NORMAL_STATE               3
#define POWER_STATUS_HW_POWER_OFF               4
#define POWER_STATUS_SW_POWER_OFF               5
#define POWER_STATUS_SW_POFF_UBOOT_PASSED       6
#define POWER_STATUS_FWUPDATING                 7
#define POWER_STATUS_REBOOT_REACHED_HALT        8
#define	POWER_STATUS_SW_POFF_REACHED_HALT       9
#define POWER_STATUS_UPS_SHUTDOWN		10
#define POWER_STATUS_UPS_SHUTDOWN_REACHED_HALT	11
#define POWER_STATUS_WOL_READY_STATE		12
#define POWER_STATUS_WOL_READY_UBOOT_PASSED	13

#define MagicKeyReboot                  0x18
#define MagicKeyRebootUbootPassed       0x3a
#define MagicKeyNormalState             0x71
#define MagicKeyHwPoff                  0x43
#define MagicKeySwPoff                  0x02
#define MagicKeySWPoffUbootPassed       0x5c
#define MagicKeyFWUpdating              0x6f
#define MagicKeyRebootReachedHalt       0x2b
#define MagicKeySWPoffReachedHalt       0x7a
#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
#define MagicKeyUpsShutdown		0x45
#define MagicKeyUpsShutdownReachedHalt	0x16
#else /* CONFIG_ARCH_FEROCEON_MV78XX0 */
#define MagicKeyUpsShutdown		0x21
#define MagicKeyUpsShutdownReachedHalt	0x32
#endif /* CONFIG_ARCH_FEROCEON_MV78XX0 */
#define MagicKeyWOLReadyState		0x75
#define MagicKeyWOLReadyUbootPasswd	0x70

/* routines are in kernel/arch/ppc/platforms/buffalo/kernevnt.c */

void kernevnt_LanAct(void *data);
void kernevnt_SwitchHubAct(const char *msg);
#ifdef CONFIG_MD
void kernevnt_RaidRecovery(int devno,int on, int isRecovery, int major, int minor);
void kernevnt_RaidScan(int devno, int on);
void kernevnt_RaidDegraded(int devno, int major, int minor);
void kernevnt_RaidReshape(int devno, int on);
#endif

// drivers/block/ll_rw_blk.c
void kernevnt_IOErr(const char *kdevname, const char *dir, sector_t sector, unsigned int errcnt);  /* 2006.9.5 :add errcnt */
void kernevnt_FlashUpdate(int on);
void kernevnt_DriveDead(const char *drvname);
void kernevnt_I2cErr(void);
void kernevnt_MiconInt(void);
void kernevnt_EnetOverload(const char *name);
void buffalo_kernevnt_queuein(const char *cmd);

#endif
