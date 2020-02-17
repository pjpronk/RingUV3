#ifndef STORAGE_H_
#define STORAGE_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "Config.h"
#include "ff_gen_drv.h"
#include "usbh_diskio.h"
#include "sd_diskio_dma.h"

extern osMessageQId StorageEvent;
extern SemaphoreHandle_t StorageSemaphore;
extern char file_buffer[4096];

#define FILEMGR_LIST_DEPDTH                      24
#define FILEMGR_FILE_NAME_SIZE                  256
#define FILEMGR_MAX_LEVEL                        3
#define FILEMGR_MAX_EXT_SIZE                     3

#define FILETYPE_DIR                             0
#define FILETYPE_FILE                            1

#define NUM_DISK_UNITS                           2
#define MSD_DISK_UNIT                            0
#define USB_DISK_UNIT                            1

typedef struct _FILELIST_LineTypeDef
{
	uint8_t               type; /* 0, file/ 1: folder */
	uint8_t               name[FILEMGR_FILE_NAME_SIZE];
}
FILELIST_LineTypeDef;

typedef struct _FILELIST_FileTypeDef
{
	FILELIST_LineTypeDef  file[FILEMGR_LIST_DEPDTH] ;
	uint16_t              ptr;
}
FILELIST_FileTypeDef;

typedef enum
{
	USBDISK_DISCONNECTION_EVENT = 1,
	USBDISK_CONNECTION_EVENT = 2,
	MSDDISK_DISCONNECTION_EVENT = 3,
	MSDDISK_CONNECTION_EVENT = 4,
}
STORAGE_EventTypeDef;

void storageInitialization(void);
uint8_t  STORAGE_GetStatus (uint8_t unit);
TCHAR   *Storage_GetDrive (uint8_t unit);
uint32_t STORAGE_GetCapacity (uint8_t unit);
uint32_t STORAGE_GetFree (uint8_t unit);
void     STORAGE_NotifyConnectionChange(uint8_t unit, uint8_t state);

#ifdef __cplusplus
}
#endif

#endif /* STORAGE_H_ */
