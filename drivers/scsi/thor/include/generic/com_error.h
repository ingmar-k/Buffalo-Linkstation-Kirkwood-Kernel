#ifndef __COM_ERROR_H__
#define __COM_ERROR_H__
#define ERR_GENERIC                              2
#define ERR_RAID                                 50
#define ERR_CORE                                 100
#define ERR_API                                  150

#define ERR_NONE                                 0
#define ERR_FAIL                                 1
/* generic error */
#define ERR_UNKNOWN                              (ERR_GENERIC + 1)
#define ERR_NO_RESOURCE                          (ERR_GENERIC + 2)
#define ERR_REQ_OUT_OF_RANGE                     (ERR_GENERIC + 3)
#define ERR_INVALID_REQUEST                      (ERR_GENERIC + 4)
#define ERR_INVALID_PARAMETER                    (ERR_GENERIC + 5)
#define ERR_INVALID_LD_ID                        (ERR_GENERIC + 6)
#define ERR_INVALID_HD_ID                        (ERR_GENERIC + 7)
#define ERR_INVALID_EXP_ID                       (ERR_GENERIC + 8)
#define ERR_INVALID_PM_ID                        (ERR_GENERIC + 9)
#define ERR_INVALID_BLOCK_ID                     (ERR_GENERIC + 10)
#define ERR_INVALID_ADAPTER_ID                   (ERR_GENERIC + 11)
#define ERR_INVALID_RAID_MODE                    (ERR_GENERIC + 12)

/* RAID errors */
#define ERR_TARGET_IN_LD_FUNCTIONAL              (ERR_RAID + 1)
#define ERR_TARGET_NO_ENOUGH_SPACE               (ERR_RAID + 2)
#define ERR_HD_IS_NOT_SPARE                      (ERR_RAID + 3)
#define ERR_HD_IS_SPARE                          (ERR_RAID + 4)
#define ERR_HD_NOT_EXIST                         (ERR_RAID + 5)
#define ERR_HD_IS_ASSIGNED_ALREADY               (ERR_RAID + 6)
#define ERR_INVALID_HD_COUNT                     (ERR_RAID + 7)
#define ERR_LD_NOT_READY                         (ERR_RAID + 8)
#define ERR_LD_NOT_EXIST                         (ERR_RAID + 9)
#define ERR_LD_IS_FUNCTIONAL                     (ERR_RAID + 10)
#define ERR_HAS_BGA_ACTIVITY                     (ERR_RAID + 11)
#define ERR_NO_BGA_ACTIVITY                      (ERR_RAID + 12)
#define ERR_BGA_RUNNING                          (ERR_RAID + 13)
#define ERR_RAID_NO_AVAILABLE_ID                 (ERR_RAID + 14)
#define ERR_LD_NO_ATAPI                          (ERR_RAID + 15)
#define ERR_INVALID_RAID6_PARITY_DISK_COUNT      (ERR_RAID + 16)
#define ERR_INVALID_BLOCK_SIZE                   (ERR_RAID + 17)
#define ERR_MIGRATION_NOT_NEED                   (ERR_RAID + 18)
#define ERR_STRIPE_BLOCK_SIZE_MISMATCH           (ERR_RAID + 19)
#define ERR_MIGRATION_NOT_SUPPORT                (ERR_RAID + 20)
#define ERR_LD_NOT_FULLY_INITED                  (ERR_RAID + 21)
#define ERR_LD_NAME_INVALID	                     (ERR_RAID + 22)

/* API errors */
#define ERR_INVALID_MATCH_ID                     (ERR_API + 1)    
#define ERR_INVALID_HDCOUNT                      (ERR_API + 2)
#define ERR_INVALID_BGA_ACTION                   (ERR_API + 3)
#define ERR_HD_IN_DIFF_CARD                      (ERR_API + 4)
#define ERR_INVALID_FLASH_TYPE                   (ERR_API + 5)
#define ERR_INVALID_FLASH_ACTION                 (ERR_API + 6)
#define ERR_TOO_FEW_EVENT                        (ERR_API + 7)
#define ERR_VD_HAS_RUNNING_OS                    (ERR_API + 8)
#define ERR_DISK_HAS_RUNNING_OS                  (ERR_API + 9)
#define ERR_COMMAND_NOT_SUPPURT                  (ERR_API + 10)
#define ERR_MIGRATION_LIMIT	                     (ERR_API + 11)

#endif /*  __COM_ERROR_H__ */
