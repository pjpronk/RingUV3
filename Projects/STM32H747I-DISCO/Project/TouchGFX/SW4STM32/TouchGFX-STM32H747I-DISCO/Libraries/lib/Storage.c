#include "Storage.h"
#include "Debug.h"

#define DECODE_MEDIA_DISCONNECTED_MESSAGE 0xFF07

#ifdef __GNUC__
#define PRAGMA_SDIO_HEAP_LOCATION
#define ATTRIBUTE_SDIO_HEAP_LOCATION __attribute__ ((section ("sdio_heap"))) __attribute__ ((aligned(4)))
#elif defined __ICCARM__
#define PRAGMA_SDIO_HEAP_LOCATION _Pragma("location=\"sdio_heap\"")
#define ATTRIBUTE_SDIO_HEAP_LOCATION
#elif defined(__ARMCC_VERSION)
#define PRAGMA_SDIO_HEAP_LOCATION
#define ATTRIBUTE_SDIO_HEAP_LOCATION __attribute__ ((section ("sdio_heap"))) __attribute__ ((aligned(4)))
#endif

FATFS USBDISK_FatFs __attribute__((aligned(4))); 	/* File system object for USB disk logical drive */
FATFS MSDDISK_FatFs __attribute__((aligned(4)));	/* File system object for SDMMC disk logical drive */
char file_buffer[4096] __attribute__((aligned(4))) = {0};

char USBDISK_Drive[4] = "1:/";       /* USB Host logical drive number */
char MSDDISK_Drive[4] = "0:/";       /* SDMMC logical drive number */

const Diskio_drvTypeDef * USBDISK_Driver = &USBH_Driver;
const Diskio_drvTypeDef * MSDDISK_Driver = &SD_Driver;

osMessageQId StorageEvent;
SemaphoreHandle_t StorageSemaphore;
USBH_HandleTypeDef hUSBHost;

static uint32_t StorageStatus[NUM_DISK_UNITS];

static void StorageThread(void const * argument);
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);
static void storageCopyResultsToUsb(void);
static void storageCopyFile(char *source_file_name, char *destination_file_name);

/**
  * @brief  Initialize the data storage hardware
  * @param  None
  * @retval None
  */
void storageInitialization(void)
{
	debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "storageInitialization\r\n");
	/* Create semaphore */
	StorageSemaphore = xSemaphoreCreateMutex();
	/* Create Storage Message Queue */
	osMessageQDef(osqueue, 30, uint16_t);
	StorageEvent = osMessageCreate(osMessageQ(osqueue), NULL);
	/**** USB HOST Initialization ****/
	/* Create USB background task */
	osThreadDef(STORAGE_Thread, StorageThread, osPriorityNormal, 0, 1024);
	osThreadCreate(osThread(STORAGE_Thread), NULL);

	/* Init Host Library */
	USBH_Init(&hUSBHost, USBH_UserProcess, 0);

	/* Add Supported Class */
	USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

	/* Start Host Process */
	USBH_Start(&hUSBHost);

	/* Enable the USB voltage level detector */
	HAL_PWREx_EnableUSBVoltageDetector();

	/**** SDMMC Initialization ****/
	/* Enable SD Interrupt mode */
	BSP_SD_ITConfig();

	/* NVIC configuration for SDMMC1 interrupts */
	HAL_NVIC_SetPriority(SDMMC1_IRQn, 4, 0);
	HAL_NVIC_EnableIRQ(SDMMC1_IRQn);

	if(BSP_SD_IsDetected())
	{
		osMessagePut(StorageEvent, MSDDISK_CONNECTION_EVENT, 0);
	}
}

/**
  * @brief  Storage get status
  * @param  unit: logical storage unit index.
  * @retval Status of the storage
  */
uint8_t STORAGE_GetStatus (uint8_t unit)
{
	return StorageStatus[unit];
}

/**
  * @brief  Get Drive Logical storage driver name
  * @param  unit: logical storage unit index.
  * @retval Pointer to drive
  */
TCHAR *Storage_GetDrive (uint8_t unit)
{
	if(unit == USB_DISK_UNIT)
	{
		return (TCHAR*)USBDISK_Drive;
	}
	else
	{
		return (TCHAR*)MSDDISK_Drive;
	}
}

/**
  * @brief  Storage get capacity
  * @param  unit: logical storage unit index
  * @retval Storage capacity
  */
uint32_t STORAGE_GetCapacity (uint8_t unit)
{
	uint32_t   tot_sect = 0;
	FATFS *fs;

	if(unit == USB_DISK_UNIT)
	{
		fs = &USBDISK_FatFs;
	}
	else
	{
		fs = &MSDDISK_FatFs;
	}
	tot_sect = (fs->n_fatent - 2) * fs->csize;
	return (tot_sect);
}

/**
  * @brief  Storage get free space
  * @param  unit: logical storage unit index.
  * @retval Free space
  */
uint32_t STORAGE_GetFree (uint8_t unit)
{
	uint32_t   fre_clust = 0;
	FATFS *fs ;
	FRESULT res = FR_INT_ERR;

	if(unit == USB_DISK_UNIT)
	{
		fs = &USBDISK_FatFs;
		res = f_getfree("1:", (DWORD *)&fre_clust, &fs);
	}
	else
	{
		fs = &MSDDISK_FatFs;
		res = f_getfree("0:", (DWORD *)&fre_clust, &fs);
	}

	if(res == FR_OK)
	{
		return (fre_clust * fs->csize);
	}
	else
	{
		return 0;
	}
}

/**
  * @brief  User Process
  * @param  phost: USB host handle
  * @param  id: USB host library user message ID
  * @retval None
  */
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
	switch (id)
	{
		case HOST_USER_SELECT_CONFIGURATION:
		break;

		case HOST_USER_DISCONNECTION:
			osMessagePut(StorageEvent, USBDISK_DISCONNECTION_EVENT, 0);
		break;

		case HOST_USER_CLASS_ACTIVE:
			osMessagePut(StorageEvent, USBDISK_CONNECTION_EVENT, 0);
		break;
	}
}

/**
  * @brief  Storage Thread that handles the storage events
  * @param  argument: pointer that is passed to the thread function as start argument.
  * @retval None
  */
static void StorageThread(void const * argument)
{
	osEvent event;

	for( ;; )
	{
		event = osMessageGet(StorageEvent, 1000);

		if(event.status == osEventMessage)
		{
			switch(event.value.v)
			{
				case USBDISK_CONNECTION_EVENT:
					/* Link the USB Host disk I/O driver */
					if(FATFS_LinkDriver((Diskio_drvTypeDef*)USBDISK_Driver, (TCHAR*)USBDISK_Drive) == 0)
					{
						if (f_mount(&USBDISK_FatFs, USBDISK_Drive, 0) == FR_OK)
						{
							StorageStatus[USB_DISK_UNIT] = 1;
							STORAGE_NotifyConnectionChange(USB_DISK_UNIT, 1);
						}
					}
				break;

				case USBDISK_DISCONNECTION_EVENT:
					if(f_mount(NULL, USBDISK_Drive, 0) == FR_OK)
					{
						if(FATFS_UnLinkDriver(USBDISK_Drive) == 0)
						{
							StorageStatus[USB_DISK_UNIT] = 0;
							STORAGE_NotifyConnectionChange (USB_DISK_UNIT, 0);
						}
					}
				break;

				case MSDDISK_CONNECTION_EVENT:
					/* Enable SD Interrupt mode */
					if(BSP_SD_Init() == MSD_OK)
					{
						if(BSP_SD_ITConfig() == MSD_OK)
						{
							/* NVIC configuration for SDMMC1 interrupts */
							HAL_NVIC_SetPriority(SDMMC1_IRQn, 4, 0);
							HAL_NVIC_EnableIRQ(SDMMC1_IRQn);

							if(FATFS_LinkDriver((Diskio_drvTypeDef*)MSDDISK_Driver, (TCHAR*)MSDDISK_Drive) == 0)
							{
								if (!f_mount(&MSDDISK_FatFs, MSDDISK_Drive, 0))
								{
									StorageStatus[MSD_DISK_UNIT] = 1;
									STORAGE_NotifyConnectionChange (MSD_DISK_UNIT, 1);
								}
							}
						}
					}
				break;

				case MSDDISK_DISCONNECTION_EVENT:
					f_mount(NULL, MSDDISK_Drive, 0);
					FATFS_UnLinkDriver(MSDDISK_Drive);
					StorageStatus[MSD_DISK_UNIT] = 0;
					BSP_SD_DeInit();
					STORAGE_NotifyConnectionChange (MSD_DISK_UNIT, 0);
				break;
			}
		}
		/* Copy the results from the SD card to the USB drive */
		storageCopyResultsToUsb();
	}
}

/**
  * @brief  Notify storage unit connection state.
  * @retval None
  */
__weak void  STORAGE_NotifyConnectionChange(uint8_t unit, uint8_t state)
{

}

/**
  * @brief  Copy the results from the SD card to the USB drive when it is available
  * @param  None
  * @retval None
  */
static void storageCopyResultsToUsb(void)
{
	/* Only continue when the USB disk is connected */
	if(STORAGE_GetStatus(USB_DISK_UNIT) == 0)
		return;
	/* Wait for the shared resource to become free */
	xSemaphoreTake(StorageSemaphore, portMAX_DELAY);

	static uint8_t first = 0;
	char source_file_name[32] = "0://RESULT/2019-01.TXT";
	char destination_file_name[32] = "1://RESULT/2019-01.TXT";
	FRESULT f_result;
	DIR directory;
	FILINFO file_info;
	FILINFO file_info_source;
	/* Create directory if it doesn't exist */
	if(f_stat("1://RESULT", &file_info) != FR_OK)
		f_mkdir("1://RESULT");
	/* Find the first file in the result directory */
	if(first == 0)
	{
		f_result = f_findfirst(&directory, &file_info, "0://RESULT", "*.txt");
		/* File is available */
		if((f_result == FR_OK) && (file_info.fname[0]))
		{
			snprintf(source_file_name, 32, "0://RESULT/%s", file_info.fname);
			snprintf(destination_file_name, 32, "1://RESULT/%s", file_info.fname);
			/* Check if file exists on the USB drive and if it has the same size */
			f_result = f_stat(destination_file_name, &file_info);
			f_stat(source_file_name, &file_info_source);

			if((file_info.fsize != file_info_source.fsize) || (f_result != FR_OK))
				storageCopyFile(source_file_name, destination_file_name);
			first = 1;
		}
		/* File is not available */
		else
		{
			f_closedir(&directory);
		}
	}
	/* Find the next file in the result directory */
	else
	{
		f_result = f_findnext(&directory, &file_info);
		/* File is available */
		if((f_result == FR_OK) && (file_info.fname[0]))
		{
			snprintf(source_file_name, 32, "0://RESULT/%s", file_info.fname);
			snprintf(destination_file_name, 32, "1://RESULT/%s", file_info.fname);
			/* Check if file exists on the USB drive and if it has the same size */
			f_result = f_stat(destination_file_name, &file_info);
			f_stat(source_file_name, &file_info_source);

			if((file_info.fsize != file_info_source.fsize) || (f_result != FR_OK))
				storageCopyFile(source_file_name, destination_file_name);
		}
		/* File is not available */
		else
		{
			first = 0;
			f_closedir(&directory);
		}
	}
	/* Free the shared resource */
	xSemaphoreGive(StorageSemaphore);
}

/**
  * @brief  Copy a file
  * @param  *source_file_name: name of the source file
  * 		*destination_file_name: name of the destination file
  * @retval None
  */
static void storageCopyFile(char *source_file_name, char *destination_file_name)
{
	if((source_file_name == NULL) || (destination_file_name == NULL))
		return;

	debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "copy %s %s\r\n", source_file_name, destination_file_name);

	FRESULT f_result;
	FIL source_file;
	FIL destination_file;
	UINT br, bw;

	if(f_open(&source_file, source_file_name, FA_READ) == FR_OK)
	{
		if(f_open(&destination_file, destination_file_name, FA_CREATE_ALWAYS | FA_WRITE) == FR_OK)
		{
		    /* Copy source to destination */
		    for (;;) {
		    	f_result = f_read(&source_file, file_buffer, sizeof(file_buffer), &br);  /* Read a chunk of source file */
		        if (f_result || br == 0) break; /* error or eof */
		        f_result = f_write(&destination_file, file_buffer, br, &bw);            /* Write it to the destination file */
		        if (f_result || bw < br) break; /* error or disk full */
		    }
		}
		else
		{
			debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "dest file open write error\r\n");
		}
	}
	else
	{
		debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "source file open read error\r\n");
	}
	f_close(&source_file);
	f_close(&destination_file);
}
