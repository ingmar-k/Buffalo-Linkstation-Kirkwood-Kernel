#if 1 //LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,4)
#include <generated/autoconf.h>
#else
#include <linux/autoconf.h>
#endif
#include <linux/types.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/module.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/version.h>
#include <asm/uaccess.h>

//#include <aes_drv.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
#include <linux/syscalls.h>
#endif

#define malloc(_size_) kmalloc(_size_,GFP_ATOMIC)

/************************************************ Headers Definitions ****************************************/
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

/********************************************************************************************************************/

static _AesActionData sActionData;
static char *scpInputDataBuffer = NULL;
char *scpAesOutputDataBuffer = NULL;

static int siIsAesRunning = -1;

extern void AesStop(void);
extern int   AesAction(char *cpInData,int reqSize,char** cpOutData,int iIVFromUser);
extern void* AesInit(int bufNum, int bufSize);
extern int AesOpenSessionAndSetKey(char *cpKey, int iKeyLength, int iDirection,int iMode);
extern void AesActionCompleted(void);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,4)
static long int
Aesdrv_ioctl(
	struct file *filp,
	unsigned long arg)
#else
static int
Aesdrv_ioctl(
	struct inode *inode,
	struct file *filp,
	unsigned long arg)
#endif
{
	int error = 0;
	//printk("%s: cmd=0x%x\n", __FUNCTION__, sActionData.sState);

	if(copy_from_user(&sActionData, (void*)arg, sizeof(_AesActionData))){
		printk("\n Copy of Action Data From Userspce to Kernel Space Failed\n");
		return -1;
	}
	switch (sActionData.sState){
    case AES_INIT:
		{
			scpInputDataBuffer =(char*) AesInit(1,(sActionData.iBufSize+16));
			if(NULL == scpInputDataBuffer){
				printk("\n AesInit Failed\n");
				return -1;
			}
			siIsAesRunning = 1;
			return 0;
		}
		break;
	case AES_OPEN_SESSION_AND_SET_KEY:
		{
			char *cpKeyData = malloc(sActionData.iInputDataLen+1);
			if(NULL == cpKeyData){
				printk("\nAllocating the Memory for the Key data Failed!!\n");
				return -1;
			}
			memset(cpKeyData,'\0',sActionData.iInputDataLen+1);
			if(copy_from_user(cpKeyData, (void*)sActionData.cpInputData, sActionData.iInputDataLen)){
				printk("\n Copy of Key Data From Userspce to Kernel Space Failed\n");
				return -1;
			}

			if(AES_ECB == sActionData.iMode){
				error = AesOpenSessionAndSetKey(cpKeyData,sActionData.iInputDataLen,sActionData.sAction,0);
			}
			else{
				error = AesOpenSessionAndSetKey(cpKeyData,sActionData.iInputDataLen,sActionData.sAction,1);
			}

			if(0 != error){
				printk("\n AesOpenSessionAndSetKey Failed\n");
				return -1;
			}
			if(cpKeyData)
				kfree(cpKeyData);
			return 0;
		}
		break;
	case AES_ACTION:
		{
			if(NULL == scpInputDataBuffer){
				printk("\n Input Data Buffer is NULL\n");
				return -1;
			}
			if(AES_CBC == sActionData.iMode){
				memcpy(scpInputDataBuffer,sActionData.cpIV, sizeof(sActionData.cpIV));
				if(copy_from_user(scpInputDataBuffer+sizeof(sActionData.cpIV),(void*)sActionData.cpInputData, sActionData.iInputDataLen)){
					printk("\n Copy of Action Data From Userspce to Kernel Space Failed\n");
					return -1;
				}
				error = AesAction(scpInputDataBuffer,sActionData.iInputDataLen,&scpAesOutputDataBuffer,1);
			}
			else{
				if(copy_from_user(scpInputDataBuffer, (void*)sActionData.cpInputData, sActionData.iInputDataLen)){
					printk("\n Copy of Action Data From Userspce to Kernel Space Failed\n");
					return -1;
				}
				error = AesAction(scpInputDataBuffer,sActionData.iInputDataLen,&scpAesOutputDataBuffer,0);
			}
			//	memset(scpAesOutputDataBuffer,0,sActionData.iInputDataLen);
			if(0 != error){
				printk("\n sActionData Failed\n");
				return -1;
			}
			if(AES_CBC == sActionData.iMode){
				if(copy_to_user(sActionData.cpOutputData, (void*)scpAesOutputDataBuffer+sizeof(sActionData.cpIV), sActionData.iOuputDataLen)){
					printk("\n Copy of Output Data to Userspce from Kernel Space Failed\n");
					return -1;
				}
			}
			else{
				if(copy_to_user(sActionData.cpOutputData, (void*)scpAesOutputDataBuffer, sActionData.iOuputDataLen)){
					printk("\n Copy of Output Data to Userspce from Kernel Space Failed\n");
					return -1;
				}
			}
			return 0;
		}
		break;
	case AES_CLOSE_SESSION:
		{
			AesActionCompleted();
			return 0;
		}
	case AES_STOP:
		{
			AesStop();
			siIsAesRunning = 0;
			return 0;
		}
		break;
	case AES_STATE:
		{
			return siIsAesRunning;
		}
	default:
		printk("%s (unknown ioctl 0x%x)\n", __FUNCTION__, sActionData.sState);
		error = EINVAL;
		break;
	}
	return(-error);
}


static int
Aesdrv_open(struct inode *inode, struct file *filp)
{
	//printk("%s()\n", __FUNCTION__);
	return(0);
}

static int
Aesdrv_release(struct inode *inode, struct file *filp)
{
	//printk("%s()\n", __FUNCTION__);
	return(0);
}


static struct file_operations Aesdrv_fops = {
	.owner = THIS_MODULE,
	.open = Aesdrv_open,
	.release = Aesdrv_release,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,3,4)
	.unlocked_ioctl = Aesdrv_ioctl,
	/* .compat_ioctl = Aesdrv_ioctl, */
#else
	.ioctl = Aesdrv_ioctl,
#endif
};

static struct miscdevice aesdev = {
	.minor = CESADEV_MINOR,
	.name = "aes",
	.fops = &Aesdrv_fops,
};

static int __init
cesadev_init(void)
{
	int rc;
	printk("%s(%p)\n", __FUNCTION__, cesadev_init);
	rc = misc_register(&aesdev);
	if (rc) {
		printk(KERN_ERR "aesdev: registration of /dev/aesdev failed\n");
		return(rc);
	}
	return(0);
}

static void __exit
Aesdrv_exit(void)
{
	printk("%s()\n", __FUNCTION__);
	misc_deregister(&aesdev);
}

module_init(cesadev_init);
module_exit(Aesdrv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ronen Shitrit");
MODULE_DESCRIPTION("Cesadev (user interface to CESA)");
