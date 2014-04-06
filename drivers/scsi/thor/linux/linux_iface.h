/*
 *
 *  Kernel/CLI interface
 *
 */

#ifndef __MV_HBA_LINUX_INTERFACE__
#define __MV_HBA_LINUX_INTERFACE__

#include "com_define.h"
#include "com_struct.h"
#include "com_api.h"
#include "com_scsi.h"
#include "com_util.h"

/*Request Structure.*/
#define SENSE_INFO_BUFFER_SIZE		32
#define MAX_COMMAND_SIZE		16

/* For Character Device Interface */
#define MV_DEVICE_MAX_SLOT 4

#define LDINFO_NUM (MAX_LD_SUPPORTED_PERFORMANCE * MAX_NUM_ADAPTERS)
#define HDINFO_NUM (MAX_DEVICE_SUPPORTED_PERFORMANCE * MAX_NUM_ADAPTERS)

typedef struct _SCSI_PASS_THROUGH_DIRECT {
	unsigned short Length;
	unsigned char  ScsiStatus;
	unsigned char  PathId;
	unsigned char  TargetId;
	unsigned char  Lun;
	unsigned char  CdbLength;
	unsigned char  SenseInfoLength;
	unsigned char  DataIn;
	unsigned long  DataTransferLength;
	unsigned long  TimeOutValue;
	void           *DataBuffer;
	unsigned long  SenseInfoOffset;
	unsigned char  Cdb[16];
}SCSI_PASS_THROUGH_DIRECT, *PSCSI_PASS_THROUGH_DIRECT;

typedef struct _SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER{
	SCSI_PASS_THROUGH_DIRECT        sptd;
	unsigned long                   Filler;
	unsigned char                   Sense_Buffer[SENSE_INFO_BUFFER_SIZE];
}SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER, *PSCSI_PASS_THROUGH_DIRECT_WITH_BUFFER;

int mv_linux_proc_info(struct Scsi_Host *pSHost, char *pBuffer, 
		       char **ppStart,  off_t offset, int length, int inout);

void IOHBARequestCallback(MV_PVOID This, PMV_Request pReq);

int mv_register_chdev(struct hba_extension *hba);
void mv_unregister_chdev(struct hba_extension *hba);

#endif /* ifndef __MV_HBA_LINUX_INTERFACE__ */
