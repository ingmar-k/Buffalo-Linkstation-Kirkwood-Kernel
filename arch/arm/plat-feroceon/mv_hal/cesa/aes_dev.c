#include "mvOs.h"

#include <linux/wait.h>
#include <linux/interrupt.h>
wait_queue_head_t   cesaTest_waitq;
spinlock_t          cesaLock;

#define CESA_TEST_LOCK(flags)       spin_lock_irqsave( &cesaLock, flags)
#define CESA_TEST_UNLOCK(flags)     spin_unlock_irqrestore( &cesaLock, flags);

#define CESA_TEST_WAIT_INIT()       init_waitqueue_head(&cesaTest_waitq)
#define CESA_TEST_WAKE_UP()         wake_up(&cesaTest_waitq)
#define CESA_TEST_WAIT(cond, ms)    wait_event_timeout(cesaTest_waitq, (cond), msecs_to_jiffies(ms))

#define CESA_TEST_TICK_GET()        jiffies
#define CESA_TEST_TICK_TO_MS(tick)  jiffies_to_msecs(tick)


#include "mvDebug.h"

#include "mvSysHwConfig.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cntmr/mvCntmr.h"
#include "cesa/mvCesa.h"
#include "cesa/mvCesaRegs.h"
#include "cesa/mvMD5.h"
#include "cesa/mvSHA1.h"
#include "ctrlEnv/sys/mvSysCesa.h"

#if defined(CONFIG_MV646xx)
#include "marvell_pic.h"
#endif


#define AES_DEF_BUF_SIZE       2048
#define AES_DEF_BUF_NUM        1
#define AES_DEF_SESSION_NUM    1

#define AES_DEF_REQ_SIZE       1


static int siSid;

MV_BUF_INFO         sAesReqBufs[AES_DEF_REQ_SIZE];

MV_CESA_COMMAND*    sAesCmdRing;
MV_CESA_RESULT      sAesResult;
int					siAesInputLen = 0;

int                 sAesTestFull = 0;

MV_BOOL             sbAesIsReady = MV_FALSE;

/*------------------------- LOCAL FUNCTIONs ---------------------------------*/
#if 0

void extractMbuf(MV_CESA_MBUF *pMbuf,
                            int offset, int size, char* cpOutputString)
{
    mvCesaCopyFromMbuf(cpOutputString, pMbuf, offset, size);
    //mvBinToHex(scpAesBinBuffer, hexStr, size);
}

static MV_STATUS    cesaSetMbuf(MV_CESA_MBUF *pMbuf,
                        char* cpInputString,
                        int offset, int reqSize)
{
    MV_STATUS   status = MV_OK;
    int         copySize, size;

    //mvHexToBin(hexString, scpAesBinBuffer, size);

    copySize = 0;
	size = reqSize;
    while(reqSize > copySize)
    {
        size = MV_MIN(size, (reqSize - copySize));

        status = mvCesaCopyToMbuf(cpInputString, pMbuf, offset+copySize, size);
        if(status != MV_OK)
        {
            mvOsPrintf("cesaSetMbuf Error: Copy %d of %d bytes to MBuf\n",
                        copySize, reqSize);
            break;
        }
        copySize += size;
    }
    pMbuf->mbufSize = offset+copySize;
    return status;
}

#endif

void AesProcessOutputData(MV_CESA_RESULT* r)
{
    sAesResult  =  *r;
    sbAesIsReady = MV_TRUE;
}


static irqreturn_t AesReadyIsr( int irq , void *dev_id /*, struct pt_regs *regs*/ )
{
    MV_U32          cause;
    MV_STATUS       status;
    MV_CESA_RESULT  result;

    /* Clear cause register */
    cause = MV_REG_READ(MV_CESA_ISR_CAUSE_REG);
    if( (cause & MV_CESA_CAUSE_ACC_DMA_ALL_MASK) == 0)
    {
        mvOsPrintf("AesReadyIsr: cause=0x%x\n", cause);
        return 1;
    }

    MV_REG_WRITE(0x9dd68, 0);
    MV_REG_WRITE(MV_CESA_ISR_CAUSE_REG, 0);

    while(MV_TRUE)
    {
        /* Get Ready requests */
		status = mvCesaReadyGet(&result);
		if(status == MV_OK)
            AesProcessOutputData(&result);

        break;
    }
    if( (sAesTestFull == 1) && (status != MV_BUSY) )
    {
        sAesTestFull = 0;
        CESA_TEST_WAKE_UP();
    }

    return 1;
}

static INLINE int   open_session(MV_CESA_OPEN_SESSION* pOs)
{
    MV_U16      sid;
    MV_STATUS   status;

    status = mvCesaSessionOpen(pOs, &sid);
	if(status != MV_OK)
    {
        mvOsPrintf("CesaTest: Can't open new session - status = 0x%x\n",
                    status);
        return -1;
    }

    return  (int)sid;
}


void close_session(int sid)
{
    MV_STATUS   status;

    status = mvCesaSessionClose(sid);
    if(status != MV_OK)
    {
        mvOsPrintf("CesaTest: Can't close session %d - status = 0x%x\n",
                    sid, status);
    }
}


int AesOpenSessionAndSetKey(char *cpKey, int iKeyLength, int iDirection, int iMode)
{
    MV_CESA_OPEN_SESSION    os;
	int i;

    os.cryptoAlgorithm = MV_CESA_CRYPTO_AES;
    os.macMode = MV_CESA_MAC_NULL;
	if(!iMode){
		os.cryptoMode = MV_CESA_CRYPTO_ECB;
	}
	else{
		os.cryptoMode = MV_CESA_CRYPTO_CBC;
	}

	if(!iDirection)
		os.direction = MV_CESA_DIR_ENCODE;
	else
		os.direction = MV_CESA_DIR_DECODE;

    os.operation = MV_CESA_CRYPTO_ONLY;

    for(i=0; i<iKeyLength; i++)
        os.cryptoKey[i] = cpKey[i];

    os.cryptoKeyLength = iKeyLength;

    os.macKeyLength = 0;
    os.digestSize = 0;

    siSid = open_session(&os);
    if(siSid == -1)
    {
        mvOsPrintf("Can't open session for test\n");
        return sAesResult.retCode;
    }
    return MV_OK;
}


void   AesActionCompleted(void)
{
    close_session(siSid);
    siSid = -1;
}


int AesCommand(MV_CESA_COMMAND* pCmd)
{
    int                 cmdReqId = 0;
    int                 i;
    MV_STATUS           rc = MV_OK;

    if(pCmd == NULL)
    {
        mvOsPrintf("AesCommand failed: pCmd=NULL\n");
        return MV_BAD_PARAM;
    }

    sbAesIsReady = MV_FALSE;


    /*mvCesaDebugStatsClear();*/
    MV_REG_WRITE(0x9dd68, 0);
    MV_REG_WRITE(MV_CESA_ISR_CAUSE_REG, 0);

    for(i=0; i<1; i++)
    {
        unsigned long flags;

        pCmd = &sAesCmdRing[cmdReqId];
        pCmd->pReqPrv = (void*)cmdReqId;

        CESA_TEST_LOCK(flags);

		rc = mvCesaAction(pCmd);
        if(rc == MV_NO_RESOURCE)
            sAesTestFull = 1;

        CESA_TEST_UNLOCK(flags);

        if(rc == MV_NO_RESOURCE)
        {
            CESA_TEST_WAIT( (sAesTestFull == 0), 100);
            if(sAesTestFull == 1)
            {

                sAesTestFull = 0;
                return MV_TIMEOUT;
            }

            CESA_TEST_LOCK(flags);

            rc = mvCesaAction(pCmd);

            CESA_TEST_UNLOCK(flags);
        }
        if( (rc != MV_OK) && (rc != MV_NO_MORE) )
        {
            mvOsPrintf("mvCesaAction failed: rc=%d\n", rc);
            return rc;
        }

        cmdReqId++;
        if(cmdReqId >= AES_DEF_REQ_SIZE)
            cmdReqId = 0;

#ifdef MV_LINUX
        /* Reschedule each 16 requests */
        if( (i & 0xF) == 0)
            schedule();
#endif
    }
    return MV_OK;
}

void* AesInit(int bufNum, int bufSize)
{
    int             numOfSessions, queueDepth, i, j, idx;
    MV_CESA_MBUF    *pMbufSrc, *pMbufDst;
    MV_BUF_INFO     *pFragsSrc, *pFragsDst;
    char            *pBuf, *pSram;
    MV_STATUS       status;
    MV_CPU_DEC_WIN  addrDecWin;
	void	*vpOutputPtr;

    sAesCmdRing = mvOsMalloc(sizeof(MV_CESA_COMMAND) * AES_DEF_REQ_SIZE);
    if(sAesCmdRing == NULL)
    {
        mvOsPrintf("testStart: Can't allocate %d bytes of memory\n",
                sizeof(MV_CESA_COMMAND) * AES_DEF_REQ_SIZE);
        return NULL;
    }
    memset(sAesCmdRing, 0, sizeof(MV_CESA_COMMAND) * AES_DEF_REQ_SIZE);

    if(bufNum == 0)
        bufNum = AES_DEF_BUF_NUM;

    if(bufSize == 0)
        bufSize = AES_DEF_BUF_SIZE;

    mvOsPrintf("CESA test started: bufNum = %d, bufSize = %d\n",
                bufNum, bufSize);



    CESA_TEST_WAIT_INIT();

    pMbufSrc = mvOsMalloc(sizeof(MV_CESA_MBUF) * AES_DEF_REQ_SIZE);
    pFragsSrc = mvOsMalloc(sizeof(MV_BUF_INFO) * bufNum * AES_DEF_REQ_SIZE);

    pMbufDst = mvOsMalloc(sizeof(MV_CESA_MBUF) * AES_DEF_REQ_SIZE);
    pFragsDst = mvOsMalloc(sizeof(MV_BUF_INFO) * bufNum * AES_DEF_REQ_SIZE);

    if( (pMbufSrc == NULL) || (pFragsSrc == NULL) ||
        (pMbufDst == NULL) || (pFragsDst == NULL) )
    {
        mvOsPrintf("testStart: Can't malloc Src and Dst pMbuf and pFrags structures.\n");
        return NULL;
    }

    memset(pMbufSrc, 0, sizeof(MV_CESA_MBUF) * AES_DEF_REQ_SIZE);
    memset(pFragsSrc, 0, sizeof(MV_BUF_INFO) * bufNum * AES_DEF_REQ_SIZE);

    memset(pMbufDst, 0, sizeof(MV_CESA_MBUF) * AES_DEF_REQ_SIZE);
    memset(pFragsDst, 0, sizeof(MV_BUF_INFO) * bufNum * AES_DEF_REQ_SIZE);

    mvOsPrintf("Cesa Test Start: pMbufSrc=%p, pFragsSrc=%p, pMbufDst=%p, pFragsDst=%p\n",
                pMbufSrc, pFragsSrc, pMbufDst, pFragsDst);

    idx = 0;
    for(i=0; i<AES_DEF_REQ_SIZE; i++)
    {
        pBuf = mvOsIoCachedMalloc(NULL, bufSize * bufNum * 2,
                                &sAesReqBufs[i].bufPhysAddr,&sAesReqBufs[i].memHandle);
		if(pBuf == NULL)
    	{
        	mvOsPrintf("testStart: Can't malloc %d bytes for pBuf\n",
                    bufSize * bufNum * 2);
        	return NULL;
    	}

        memset(pBuf, 0, bufSize * bufNum * 2);
        mvOsCacheFlush(NULL, pBuf, bufSize * bufNum * 2);

        if(pBuf == NULL)
        {
            mvOsPrintf("AesInit: Can't allocate %d bytes for req_%d buffers\n",
                        bufSize * bufNum * 2, i);
            return NULL;
        }

        sAesReqBufs[i].bufVirtPtr = pBuf;
        sAesReqBufs[i].bufSize =  bufSize * bufNum * 2;

        sAesCmdRing[i].pSrc = &pMbufSrc[i];
        sAesCmdRing[i].pSrc->pFrags = &pFragsSrc[idx];
        sAesCmdRing[i].pSrc->numFrags = bufNum;
        sAesCmdRing[i].pSrc->mbufSize = 0;

        sAesCmdRing[i].pDst = &pMbufDst[i];
        sAesCmdRing[i].pDst->pFrags = &pFragsDst[idx];
        sAesCmdRing[i].pDst->numFrags = bufNum;
        sAesCmdRing[i].pDst->mbufSize = 0;

        for(j=0; j<bufNum; j++)
        {
            sAesCmdRing[i].pSrc->pFrags[j].bufVirtPtr = pBuf;
            sAesCmdRing[i].pSrc->pFrags[j].bufSize = bufSize;
            pBuf += bufSize;
            sAesCmdRing[i].pDst->pFrags[j].bufVirtPtr = pBuf;
            sAesCmdRing[i].pDst->pFrags[j].bufSize = bufSize;
            pBuf += bufSize;
        }
	idx += bufNum;
    }

	vpOutputPtr = sAesCmdRing[0].pSrc->pFrags[0].bufVirtPtr;

    if (mvCpuIfTargetWinGet(CRYPT_ENG, &addrDecWin) == MV_OK)
        pSram = (char*)addrDecWin.addrWin.baseLow;
    else
    {
        mvOsPrintf("mvCesaInit: ERR. mvCpuIfTargetWinGet failed\n");
        return NULL;
    }

#ifdef MV_CESA_NO_SRAM
    pSram = mvOsMalloc(4*1024+8);
    if(pSram == NULL)
    {
        mvOsPrintf("CesaTest: can't allocate %d bytes for SRAM simulation\n",
                4*1024+8);
        return NULL;
    }
    pSram = (MV_U8*)MV_ALIGN_UP((MV_U32)pSram, 8);
#endif /* MV_CESA_NO_SRAM */

    numOfSessions = AES_DEF_SESSION_NUM;
    queueDepth = 1;

    status = mvCesaInit(numOfSessions, queueDepth, pSram, NULL);
    if(status != MV_OK)
    {
        mvOsPrintf("mvCesaInit is Failed: status = 0x%x\n", status);
        return NULL;
    }

    MV_REG_WRITE( MV_CESA_ISR_CAUSE_REG, 0);
    MV_REG_WRITE( MV_CESA_ISR_MASK_REG, MV_CESA_CAUSE_ACC_DMA_MASK);

    if( request_irq(CESA_IRQ, AesReadyIsr, (IRQF_DISABLED) , "cesa_test", NULL ) )
    {
        mvOsPrintf( "cannot assign irq\n" );
        return NULL;
    }
    spin_lock_init( &cesaLock );
	return vpOutputPtr;
}

int   AesAction(char *cpInData,int reqSize,char** cpOutData,int iIVFromUser)
{

    MV_CESA_COMMAND         cmd;
    MV_STATUS               status;
	int count;

	if(cpInData != (char*)sAesCmdRing[0].pSrc->pFrags[0].bufVirtPtr){
		printk("\n AesAction: Error in Input data pointer received!!\n");
		return -1;
	}
    memset(&cmd, 0, sizeof(cmd));
    cmd.sessionId = siSid;

    cmd.cryptoLength = reqSize;
    cmd.macLength = reqSize;

	cmd.pSrc = (MV_CESA_MBUF*)(sAesCmdRing[0].pSrc);
	cmd.pDst = (MV_CESA_MBUF*)(sAesCmdRing[0].pDst);
	if(iIVFromUser){
		cmd.ivFromUser = 1;
		cmd.ivOffset = 0;
		cmd.cryptoOffset = MV_CESA_AES_BLOCK_SIZE;
		cmd.pSrc->mbufSize = reqSize+MV_CESA_AES_BLOCK_SIZE;
		cmd.pDst->mbufSize = reqSize+MV_CESA_AES_BLOCK_SIZE;
		memset(sAesCmdRing[0].pDst->pFrags[0].bufVirtPtr,0,reqSize+MV_CESA_AES_BLOCK_SIZE);
	}
	else{
	    cmd.cryptoOffset = 0;
		cmd.pSrc->mbufSize = reqSize;
		cmd.pDst->mbufSize = reqSize;
		memset(sAesCmdRing[0].pDst->pFrags[0].bufVirtPtr,0,reqSize);
	}

	memcpy(&sAesCmdRing[0], &cmd, sizeof(cmd));

    status = AesCommand(&cmd);

    if(status != MV_OK)
        return status;

    /* Wait when all callbacks is received */
    count = 0;
    while(sbAesIsReady == MV_FALSE)
    {
        mvOsSleep(1);
        count++;
        if(count > 10000)
        {
            mvOsPrintf("AesAction: Timeout occured\n");
            return MV_TIMEOUT;
        }
    }

	*cpOutData = sAesCmdRing[0].pDst->pFrags[0].bufVirtPtr;
    return MV_OK;
}

void AesStop(void)
{
    MV_CESA_MBUF    *pMbufSrc, *pMbufDst;
    MV_BUF_INFO     *pFragsSrc, *pFragsDst;
    int             i;

	MV_REG_WRITE( MV_CESA_ISR_MASK_REG, 0);
	MV_REG_WRITE( MV_CESA_ISR_CAUSE_REG, 0);


	if( MV_OK != mvCesaFinish() ) {
           printk("%s,%d: mvCesaFinish Failed. \n", __FILE__, __LINE__);
	}
    /* Release all allocated memories */
    pMbufSrc = (MV_CESA_MBUF*)(sAesCmdRing[0].pSrc);
    pFragsSrc = sAesCmdRing[0].pSrc->pFrags;

    pMbufDst = (MV_CESA_MBUF*)(sAesCmdRing[0].pDst);
    pFragsDst = sAesCmdRing[0].pDst->pFrags;

    mvOsFree(pMbufSrc);
    mvOsFree(pMbufDst);
    mvOsFree(pFragsSrc);
    mvOsFree(pFragsDst);

    for(i=0; i<AES_DEF_REQ_SIZE; i++)
    {
        mvOsIoCachedFree(NULL, sAesReqBufs[i].bufSize,
                     sAesReqBufs[i].bufPhysAddr, sAesReqBufs[i].bufVirtPtr,sAesReqBufs[i].memHandle);
    }

	free_irq(CESA_IRQ, NULL);

}
