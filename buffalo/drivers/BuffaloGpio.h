#ifndef _BUFFALO_GPIO_H_
#define _BUFFALO_GPIO_H_

#include "mvCommon.h"
#include "mvOs.h"

#if defined(CONFIG_BUFFALO_PLATFORM)
extern u8 use_slide_power;

#if defined(CONFIG_ARCH_FEROCEON_MV78XX0)
#include "boardEnv/buffalo/buffalo78100BoardEnv.h"
#include "boardEnv/mvBoardEnvSpec.h"

MV_32 mvBoardGpioPinNumGet(MV_BOARD_GPP_CLASS, MV_U32);
#endif

#define BIT_PWR_LED		mvBoardGpioPinNumGet(BOARD_GPP_PWR_LED, 0)
#define BIT_INFO_LED		mvBoardGpioPinNumGet(BOARD_GPP_INFO_LED, 0)
#define BIT_ALARM_LED		mvBoardGpioPinNumGet(BOARD_GPP_ALARM_LED, 0)
#define BIT_FUNC_LED		mvBoardGpioPinNumGet(BOARD_GPP_FUNC_LED, 0)
#define BIT_FUNC_RED_LED	mvBoardGpioPinNumGet(BOARD_GPP_FUNC_RED_LED, 0)
#define BIT_FAN_LOW		mvBoardGpioPinNumGet(BOARD_GPP_FAN_LOW, 0)
#define BIT_FAN_HIGH		mvBoardGpioPinNumGet(BOARD_GPP_FAN_HIGH, 0)
#define BIT_FAN_LOCK		mvBoardGpioPinNumGet(BOARD_GPP_FAN_LOCK, 0)
#define BIT_PWR_SW		mvBoardGpioPinNumGet(BOARD_GPP_PWR_SW, 0)
#define BIT_PWRAUTO_SW		mvBoardGpioPinNumGet(BOARD_GPP_PWRAUTO_SW, 0)
#define BIT_FUNC_SW		mvBoardGpioPinNumGet(BOARD_GPP_FUNC_SW, 0)
#define BIT_INIT_SW		mvBoardGpioPinNumGet(BOARD_GPP_INIT_SW, 0)
#define BIT_MC_IRQ		mvBoardGpioPinNumGet(BOARD_GPP_MC_IRQ, 0)
#define BIT_ETH_LED		(-1)
#define BIT_UART_EN		mvBoardGpioPinNumGet(BOARD_GPP_UART_EN, 0)

#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
  #define BIT_UPS_DCD		20
  #define BIT_UPS_DTR		21
  #define BIT_UPS_DSR		22
  #define BIT_UPS_RI		23
  #define BIT_UPS_TXD		24
  #define BIT_UPS_RXD		25
  #define BIT_UPS_CTS		26
  #define BIT_UPS_RTS		27

  #define BUFFALO_UART_MCR_ST	0x0100
  #define BUFFALO_UART_MSR_SR	0x0200
#endif

#define BUFFALO_GPIO_FAN_STOP	(0)
#define BUFFALO_GPIO_FAN_SLOW	(1)
#define BUFFALO_GPIO_FAN_FAST	(2)
#define BUFFALO_GPIO_FAN_FULL	(3)

// prototypes.
void BuffaloGpio_CPUInterruptsSetup(void);
void BuffaloGpio_CPUInterruptsClear(void);

void BuffaloGpio_CPUInterruptsSetupAutopower(void);
void BuffaloGpio_CPUInterruptsClearAutopower(void);


void BuffaloGpio_CPUInterruptsSetupInit(void);
void BuffaloGpio_CPUInterruptsClearInit(void);


void BuffaloGpio_CPUInterruptsSetupFunc(void);
void BuffaloGpio_CPUInterruptsClearFunc(void);

unsigned int buffalo_gpio_read(void);
unsigned int buffalo_gpio_blink_reg_read(void);
void BuffaloGpio_Init(void);
void BuffaloGpio_LedEnable(int);
void BuffaloGpio_LedDisable(int);
void BuffaloGpio_LedInvert(int);
void BuffaloGpio_LedBlinkEnable(int);
void BuffaloGpio_LedBlinkDisable(int);

void BuffaloGpio_HddPowerOff(int);
void BuffaloGpio_HddPowerOn(int);


void BuffaloGpio_UsbPowerOff(int);
void BuffaloGpio_UsbPowerOn(int);

unsigned int BuffaloGpio_GetAutoPowerStatus(void);
void BuffaloGpio_CpuReset(void);
void BuffaloGpio_EthLedOn(void);
void BuffaloGpio_EthLedOff(void);
void BuffaloGpio_AllLedOff(void);
void BuffaloGpio_AllLedOn(void);
uint8_t BuffaloGpio_ChangePowerStatus(uint8_t);


void BuffaloGpio_FanControlWrite(unsigned int);
unsigned int BuffaloGpio_FanControlRead(void);

#define BuffaloGpio_FanStop()			BuffaloGpio_FanControlWrite(BUFFALO_GPIO_FAN_STOP)
#define BuffaloGpio_FanSlow()			BuffaloGpio_FanControlWrite(BUFFALO_GPIO_FAN_SLOW)
#define BuffaloGpio_FanFast()			BuffaloGpio_FanControlWrite(BUFFALO_GPIO_FAN_FAST)
#define BuffaloGpio_FanFull()			BuffaloGpio_FanControlWrite(BUFFALO_GPIO_FAN_FULL)


// convenient macros.

#define BuffaloGpio_AlarmLedEnable()	 	BuffaloGpio_LedEnable(BIT_ALARM_LED)
#define BuffaloGpio_AlarmLedDisable()		BuffaloGpio_LedDisable(BIT_ALARM_LED)
#define BuffaloGpio_AlarmLedBlinkEnable()	BuffaloGpio_LedBlinkEnable(BIT_ALARM_LED)
#define BuffaloGpio_AlarmLedBlinkDisable()	BuffaloGpio_LedBlinkDisable(BIT_ALARM_LED)



#define BuffaloGpio_InfoLedEnable()		BuffaloGpio_LedEnable(BIT_INFO_LED)
#define BuffaloGpio_InfoLedDisable()		BuffaloGpio_LedDisable(BIT_INFO_LED)
#define BuffaloGpio_InfoLedBlinkEnable()	BuffaloGpio_LedBlinkEnable(BIT_INFO_LED)
#define BuffaloGpio_InfoLedBlinkDisable()	BuffaloGpio_LedBlinkDisable(BIT_INFO_LED)



#define BuffaloGpio_PowerLedEnable()		BuffaloGpio_LedEnable(BIT_PWR_LED)
#define BuffaloGpio_PowerLedDisable()		BuffaloGpio_LedDisable(BIT_PWR_LED)
#define BuffaloGpio_PowerLedBlinkEnable()	BuffaloGpio_LedBlinkEnable(BIT_PWR_LED)
#define BuffaloGpio_PowerLedBlinkDisable()	BuffaloGpio_LedBlinkDisable(BIT_PWR_LED)



#define BuffaloGpio_FuncLedEnable()		BuffaloGpio_LedEnable(BIT_FUNC_LED)
#define BuffaloGpio_FuncLedDisable()		BuffaloGpio_LedDisable(BIT_FUNC_LED)
#define BuffaloGpio_FuncLedBlinkEnable()	BuffaloGpio_LedBlinkEnable(BIT_FUNC_LED)
#define BuffaloGpio_FuncLedBlinkDisable()	BuffaloGpio_LedBlinkDisable(BIT_FUNC_LED)


void BuffaloGpio_MiconIntSetup(void);
void BuffaloGpio_ClearMiconInt(void);

void BuffaloGpio_RtcIntSetup(void);
void BuffaloPrintGpio(void);


void BuffaloGpio_UPSPortEnable(void);
void BuffaloGpio_UPSPortDisable(void);
unsigned int BuffaloGpio_UPSPortScan(void); // for debug

void BuffaloGpio_UPSOmronBLEnable(void);
void BuffaloGpio_UPSOmronBLDisable(void);
unsigned int BuffaloGpio_UPSOmronBLGetStatus(void);
unsigned int BuffaloGpio_UPSOmronBLUseStatus(void);

void BuffaloRtc_UPSRecoverInit(void);
void BuffaloRtc_UPSRecoverEnable(int8_t);
void BuffaloRtc_UPSRecoverDisable(void);
int BuffaloRtc_UPSRecoverReadStatus(void);

#ifdef CONFIG_ARCH_FEROCEON_MV78XX0
unsigned int BuffaloGpio_ReadMSR_UPSPort(void);
void BuffaloGpio_WriteMCR_UPSPort(unsigned int);
#endif

int BuffaloReadPmGpio(uint32_t *value);
int BuffaloWritePmGpio(uint32_t *value);
int Buffalo_has_PM(void);

MV_BOOL bfGppInRegBitTest(MV_32 bit);
MV_BOOL bfGppOutRegBitTest(MV_32 bit);
MV_VOID bfGppOutRegBitSet(MV_32 bit);
MV_VOID bfGppOutRegBitClr(MV_32 bit);
MV_VOID bfGppOutRegBitInv(MV_32 bit);
MV_BOOL bfGppBlinkRegBitTest(MV_32 bit);
MV_VOID bfGppBlinkRegBitSet(MV_32 bit);
MV_VOID bfGppBlinkRegBitClr(MV_32 bit);
MV_BOOL bfGppPolarityRegBitTest(MV_U32 bit);
MV_VOID bfGppPolarityRegBitInv(MV_32 bit);
MV_BOOL bfGppOutEnableRegBitTest(MV_32 bit);
MV_VOID bfGppOutEnableRegBitSet(MV_32 bit);
MV_VOID bfGppOutEnableRegBitClr(MV_32 bit);
MV_VOID bfGppOutRegBitAssert(MV_32 bit);
MV_VOID bfGppOutRegBitNagate(MV_32 bit);
MV_BOOL bfIsGppOutRegBitAssert(MV_32 bit);
MV_U32 bfRegGet(MV_U32 regOffs, MV_U32 mask);
MV_VOID bfRegSet(MV_U32 regOffs, MV_U32 mask, MV_U32 value);
MV_VOID bfRegInv(MV_U32 regOffs, MV_U32 mask);
int bfIsSupportMicon(void);

#define RECOVER_TARGET_UPS_APC	1
#define RECOVER_TARGET_UPS_OMR	2
#define RECOVER_TARGET_UPS_USB	3

#endif // end of CONFIG_BUFFALO_PLATFORM
#endif

