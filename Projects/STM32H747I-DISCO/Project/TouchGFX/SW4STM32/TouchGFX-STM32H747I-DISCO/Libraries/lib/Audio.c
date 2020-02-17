#include "Audio.h"
#include "cmsis_os.h"
#include "Debug.h"

static FIL wav_file;
osMessageQId AudioEvent = 0;
static osThreadId AudioThreadId = 0;
extern SAI_HandleTypeDef haudio_out_sai;

static void Audio_Thread(void const * argument);

#ifdef __GNUC__
#define PRAGMA_AUDIO_BUFFERS_LOCATION
#define ATTRIBUTE_AUDIO_BUFFERS_LOCATION __attribute__ ((section ("audio_buffers"))) __attribute__ ((aligned(4)))
#elif defined __ICCARM__
#define PRAGMA_AUDIO_BUFFERS_LOCATION _Pragma("location=\"audio_buffers\"")
#define ATTRIBUTE_AUDIO_BUFFERS_LOCATION
#elif defined(__ARMCC_VERSION)
#define PRAGMA_AUDIO_BUFFERS_LOCATION
#define ATTRIBUTE_AUDIO_BUFFERS_LOCATION __attribute__ ((section ("audio_buffers"))) __attribute__ ((aligned(4)))
#endif

AUDIO_ProcessTypdef haudio __attribute__ ((aligned(4)));

/**
  * @brief  Initialize the audio hardware
  * @param  None
  * @retval None
  */
void audioInitialization(void)
{
	debug(DEBUG_ENABLED, DEBUG_LEVEL_INFORMATION, "audioInitialization\r\n");

	BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_BOTH, 100, I2S_AUDIOFREQ_48K);
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_13);
	/* Initialize internal audio structure */
	haudio.state  = AUDIOPLAYER_STOP;
	haudio.mute   = MUTE_OFF;
	haudio.volume = 100;
	haudio.freq   = I2S_AUDIOFREQ_48K;

	/* Create Audio Queue */
	osMessageQDef(AUDIO_Queue, 10, uint16_t);
	AudioEvent = osMessageCreate (osMessageQ(AUDIO_Queue), NULL);
	/* Create Audio task */
	osThreadDef(osAudio_Thread, Audio_Thread, osPriorityNormal, 0, 1024);
	AudioThreadId = osThreadCreate (osThread(osAudio_Thread), NULL);
}

/**
  * @brief  Play a WAV file
  * @param  *filename: name of the WAV file
  * @retval None
  */
void audioPlayFile(const char* filename)
{
	if(filename == NULL)
		return;

	AUDIOPLAYER_Stop();
	AUDIOPLAYER_SelectFile((uint8_t*)filename);
	AUDIOPLAYER_Play(44100);
}

/**
  * @brief  Is audio currently playing?
  * @param  None
  * @retval 0: no audio is playing, 1: audio is playing
  */
uint8_t isAudioPlaying(void)
{
	return (haudio.state == AUDIOPLAYER_PLAY);
}

/**
  * @brief  Get the audio state
  * @param  None
  * @retval Audio state
  */
AUDIOPLAYER_StateTypdef  AUDIOPLAYER_GetState(void)
{
	return haudio.state;
}

/**
  * @brief  Get the audio volume
  * @param  None
  * @retval Audio volume
  */
uint32_t  AUDIOPLAYER_GetVolume(void)
{
	return haudio.volume;
}

/**
  * @brief  Set audio volume
  * @param  Volume: Volume level to be set in percentage (0..100)
  * @retval Audio state
  */
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_SetVolume(uint32_t volume)
{
	if(BSP_AUDIO_OUT_SetVolume(volume) == AUDIO_OK)
	{
		haudio.volume = volume;
		return AUDIOPLAYER_ERROR_NONE;
	}
	else
	{
		return AUDIOPLAYER_ERROR_HW;
	}
}

/**
  * @brief  Play audio stream
  * @param  frequency: Audio frequency used to play the audio stream.
  * @retval Audio state
  */
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_Play(uint32_t frequency)
{
	uint32_t numOfReadBytes;
	haudio.state = AUDIOPLAYER_PLAY;

	/* Fill whole buffer @ first time */
	if(f_read(&wav_file,
	&haudio.buff[0],
	AUDIO_OUT_BUFFER_SIZE,
	(void *)&numOfReadBytes) == FR_OK)
	{
		if(numOfReadBytes != 0)
		{
			BSP_AUDIO_OUT_Pause();
			memset(haudio.buff, 0, AUDIO_OUT_BUFFER_SIZE);

			if(haudio.freq != frequency )
			{
				BSP_AUDIO_OUT_SetFrequency(frequency);
				haudio.freq = frequency;
			}
			BSP_AUDIO_OUT_SetMute(MUTE_OFF);
			BSP_AUDIO_OUT_Play((uint16_t*)&haudio.buff[0], AUDIO_OUT_BUFFER_SIZE);
			return AUDIOPLAYER_ERROR_NONE;
		}
	}
	return AUDIOPLAYER_ERROR_IO;
}

/**
  * @brief  Audio player DeInit
  * @param  None
  * @retval Audio state
  */
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_DeInit(void)
{
	haudio.state = AUDIOPLAYER_STOP;

	BSP_AUDIO_OUT_Stop(CODEC_PDWN_HW);
	BSP_AUDIO_OUT_DeInit();

	f_close(&wav_file);

	if(AudioEvent != 0)
	{
		vQueueDelete(AudioEvent);
		AudioEvent = 0;
	}
	if(AudioThreadId != 0)
	{
		osThreadTerminate(AudioThreadId);
		AudioThreadId = 0;
	}
	return AUDIOPLAYER_ERROR_NONE;
}

/**
  * @brief  Stop audio stream
  * @param  None
  * @retval Audio state
  */
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_Stop(void)
{
	BSP_AUDIO_OUT_SetMute(MUTE_ON);
	haudio.state = AUDIOPLAYER_STOP;
	BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
	f_close(&wav_file);

	return AUDIOPLAYER_ERROR_NONE;
}


/**
  * @brief  Pause Audio stream
  * @param  None
  * @retval Audio state
  */
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_Pause(void)
{
	haudio.state = AUDIOPLAYER_PAUSE;
	BSP_AUDIO_OUT_Pause();
	return AUDIOPLAYER_ERROR_NONE;
}


/**
  * @brief  Resume Audio stream
  * @param  None
  * @retval Audio state
  */
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_Resume(void)
{
	haudio.state = AUDIOPLAYER_PLAY;
	BSP_AUDIO_OUT_Resume();
	return AUDIOPLAYER_ERROR_NONE;
}
/**
  * @brief  Sets audio stream position
  * @param  position: stream position
  * @retval Audio state
  */
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_SetPosition(uint32_t position)
{
	uint64_t file_pos;

	file_pos = (f_size(&wav_file) / 128 / 100);
	file_pos *= (position * 128);
	f_lseek(&wav_file, file_pos);

	return AUDIOPLAYER_ERROR_NONE;
}

/**
  * @brief  Sets the volume at mute
  * @param  state: mute setting (MUTE_ON, MUTE_OFF)
  * @retval Audio state
  */
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_Mute(uint8_t state)
{
   BSP_AUDIO_OUT_SetMute(state);
   return AUDIOPLAYER_ERROR_NONE;
}

/**
  * @brief  Get the WAV file information.
  * @param  file: WAV file
  * @param  info: pointer to WAV file structure
  * @retval Audio state
  */
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_GetFileInfo(uint8_t* file, WAV_InfoTypedef* info)
{
	uint32_t numOfReadBytes;
	AUDIOPLAYER_ErrorTypdef ret = AUDIOPLAYER_ERROR_IO;

	if( f_open(&wav_file, (char *)file, FA_OPEN_EXISTING | FA_READ) == FR_OK)
	{
		/* Fill the buffer to Send */
		if(f_read(&wav_file, info, sizeof(WAV_InfoTypedef), (void *)&numOfReadBytes) == FR_OK)
		{
			if((info->ChunkID == 0x46464952) && (info->AudioFormat == 1))
			{
				ret = AUDIOPLAYER_ERROR_NONE;
			}
		}
		f_close(&wav_file);
	}
	return ret;
}

/**
  * @brief  Select WAV file
  * @param  file: WAV file
  * @retval Audio state
  */
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_SelectFile(uint8_t * file)
{
	AUDIOPLAYER_ErrorTypdef ret = AUDIOPLAYER_ERROR_IO;
	if( f_open(&wav_file, (char *)file, FA_OPEN_EXISTING | FA_READ) == FR_OK)
	{
		f_lseek(&wav_file, sizeof(WAV_InfoTypedef));
		ret = AUDIOPLAYER_ERROR_NONE;
	}
	return ret;
}

/**
  * @brief  Get WAV file progress
  * @param  None
  * @retval file progress
  */
uint32_t AUDIOPLAYER_GetProgress(void)
{
	return f_tell(&wav_file);
}

/**
  * @brief  Manages the DMA Transfer complete interrupt
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
	if(haudio.state == AUDIOPLAYER_PLAY)
	{
		BSP_AUDIO_OUT_ChangeBuffer((uint16_t*)&haudio.buff[0], AUDIO_OUT_BUFFER_SIZE /2);
		osMessagePut ( AudioEvent, PLAY_BUFFER_OFFSET_FULL, 0);
	}
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
	if(haudio.state == AUDIOPLAYER_PLAY)
	{
		BSP_AUDIO_OUT_ChangeBuffer((uint16_t*)&haudio.buff[AUDIO_OUT_BUFFER_SIZE /2], AUDIO_OUT_BUFFER_SIZE /2);
		osMessagePut ( AudioEvent, PLAY_BUFFER_OFFSET_HALF, 0);
	}
}

/**
  * @brief  Manages the DMA FIFO error interrupt
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
	haudio.state = AUDIOPLAYER_ERROR;
}

/**
  * @brief  Audio task
  * @param  argument: pointer that is passed to the thread function as start argument
  * @retval None
  */
static void Audio_Thread(void const * argument)
{
	uint32_t numOfReadBytes;
	osEvent event;

	for(;;)
	{
		event = osMessageGet(AudioEvent, osWaitForever);

		if(event.status == osEventMessage)
		{
			if(haudio.state == AUDIOPLAYER_PLAY)
			{
				switch(event.value.v)
				{
					case PLAY_BUFFER_OFFSET_HALF:
						if(f_read(&wav_file,
						&haudio.buff[0],
						AUDIO_OUT_BUFFER_SIZE/2,
						(void *)&numOfReadBytes) == FR_OK)
						{
							if(numOfReadBytes == 0)
							{
								AUDIOPLAYER_NotifyEndOfFile();
							}
						}
						else
						{
							AUDIOPLAYER_NotifyError();
						}
					break;

					case PLAY_BUFFER_OFFSET_FULL:
						if(f_read(&wav_file,
						&haudio.buff[AUDIO_OUT_BUFFER_SIZE/2],
						AUDIO_OUT_BUFFER_SIZE/2,
						(void *)&numOfReadBytes) == FR_OK)
						{
							if(numOfReadBytes == 0)
							{
								AUDIOPLAYER_NotifyEndOfFile();
							}
						}
						else
						{
							AUDIOPLAYER_NotifyError();
						}
					break;

					default:
					break;
				}
			}
		}
	}
}

/**
  * @brief  Notify end of playing
  * @param  None
  * @retval None
  */
__weak AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_NotifyEndOfFile(void)
{
	return AUDIOPLAYER_ERROR_NONE;
}

/**
  * @brief  Notify audio error
  * @param  None
  * @retval None
  */
__weak AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_NotifyError(void)
{
	return AUDIOPLAYER_Stop();
}
/**
  * @brief  This function handles DMA2 Stream 5 interrupt request
  * @param  None
  * @retval None
  */

void AUDIO_OUT_SAIx_DMAx_IRQHandler(void)
{
	HAL_DMA_IRQHandler(haudio_out_sai.hdmatx);
}
