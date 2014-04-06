#ifndef _CESA_DEV_H_
#define _CESA_DEV_H_

#define AES_BLOCK_SIZE 16

typedef enum {
	AES_INIT = 0,
	AES_OPEN_SESSION_AND_SET_KEY,
	AES_ACTION,
	AES_CLOSE_SESSION,
	AES_STOP,
	AES_STATE
}_AesStates;

typedef enum{
	AES_ENCRYPT = 0,
	AES_DECRYPT = 1
}_AesAction;

typedef enum{
	AES_ECB,
	AES_CBC
}_AesMode;

typedef struct AesActionData{
	int sState;
	int sAction;
	int iBufSize;
	int iMode;
	char cpIV[AES_BLOCK_SIZE];
	unsigned int iInputDataLen;
	char *cpInputData;
	unsigned int iOuputDataLen;
	char *cpOutputData;
}_AesActionData;

typedef struct HardwareAesKey{
	unsigned char cpKey[16];
	int iKeySet;
	int iModeSet; /* 0 - ECB, 1 - CBC */
}_HardwareAesKey;

typedef union{
	_HardwareAesKey HWAesKey;
}_HWAesKey;

int AesHardware128Init(unsigned char *pKey,int keyLen,_HWAesKey *sHwAesKey);
int AesHardware128Encrypt(unsigned char* pSrc,unsigned char* pDst,int aLen,char* cpIV,_HWAesKey *sHwAesKey);
int AesHardware128Decrypt(unsigned char* pSrc,unsigned char* pDst,int aLen,char *cpIV,_HWAesKey *sHwAesKey);
int AesHardware128Stop(void);
int AesHardware128CloseSession(void);
#endif /* _CESA_DEV_H_ */
