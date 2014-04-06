#if defined(__BUFFALOWOL_H__)
#else
#define __BUFFALOWOL_H__

#define FLAG_BUFFALO_WOL_INTERRUPT_ENABLE       1
#define FLAG_BUFFALO_WOL_INTERRUPT_DISABLE      0

#if defined(__KERNEL__) && !defined(MV_UBOOT)
void bfResetEthPhy(MV_U32 ethPortNum);
void bfControlWolInterrupt(MV_U32 ethPortNum, MV_U8 ope);
MV_BOOL bfIsSupportWol(MV_U32 ethPortNum);
MV_BOOL bfSetWolFrameWaitMode(MV_U32 ethPortNum, int debug);

#else
/* on u-boot */
void bfResetEthPhy(MV_U32 ethPortNum);
void bfControlWolInterrupt(MV_U32 ethPortNum, MV_U8 ope);
MV_BOOL bfIsSupportWol(MV_U32 ethPortNum);
MV_BOOL bfSetWolMagicWaitMode(MV_U32 ethPortNum);
MV_BOOL bfSetWolFrameWaitMode(MV_U32 ethPortNum, int debug);
MV_U16 bfGetWolInterruptStatus(MV_U32 ethPortNum, int debug);
void bfPhyPowerUp(MV_U32 ethPortNum);
char *bfGetWolPatternString(MV_U16 wol_stat);
#endif // defined(__KERNEL__) && !defined(MV_UBOOT)

#endif
