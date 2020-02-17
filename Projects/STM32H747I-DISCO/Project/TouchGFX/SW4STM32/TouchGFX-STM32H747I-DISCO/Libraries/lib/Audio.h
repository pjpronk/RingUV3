#ifndef AUDIO_H_
#define AUDIO_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "Config.h"
#include "stm32h747i_discovery_audio.h"
#include "ff_gen_drv.h"

extern osMessageQId AudioEvent;

#define MUTE_OFF                      0x00
#define MUTE_ON                       0x01
#define AUDIO_OUT_BUFFER_SIZE         (8 * 1024)

typedef enum
{
	AUDIOPLAYER_ERROR_NONE = 0,
	AUDIOPLAYER_ERROR_IO,
	AUDIOPLAYER_ERROR_HW,
	AUDIOPLAYER_ERROR_MEM,
	AUDIOPLAYER_ERROR_FORMAT_NOTSUPPORTED,
}AUDIOPLAYER_ErrorTypdef;

#define AUDIOPLAYER_StateTypdef OUT_StateTypdef

typedef enum
{
	BUFFER_OFFSET_NONE = 0,
	REC_BUFFER_OFFSET_HALF,
	REC_BUFFER_OFFSET_FULL,
	PLAY_BUFFER_OFFSET_HALF,
	PLAY_BUFFER_OFFSET_FULL,
}
BUFFER_StateTypeDef;

typedef enum
{
	AUDIOPLAYER_STOP = 0,
	AUDIOPLAYER_START,
	AUDIOPLAYER_PLAY,
	AUDIOPLAYER_PAUSE,
	AUDIOPLAYER_EOF,
	AUDIOPLAYER_ERROR,
}OUT_StateTypdef;

typedef enum {
	BUFFER_EMPTY = 0,
	BUFFER_FULL,
}WR_BUFFER_StateTypeDef;

typedef struct
{
	uint8_t         buff[AUDIO_OUT_BUFFER_SIZE];
	uint32_t        volume;
	uint32_t        mute;
	uint32_t        freq;
	OUT_StateTypdef state;
}AUDIO_ProcessTypdef;

typedef struct
{
	uint32_t   ChunkID;       /* 0 */
	uint32_t   FileSize;      /* 4 */
	uint32_t   FileFormat;    /* 8 */
	uint32_t   SubChunk1ID;   /* 12 */
	uint32_t   SubChunk1Size; /* 16*/
	uint16_t   AudioFormat;   /* 20 */
	uint16_t   NbrChannels;   /* 22 */
	uint32_t   SampleRate;    /* 24 */

	uint32_t   ByteRate;      /* 28 */
	uint16_t   BlockAlign;    /* 32 */
	uint16_t   BitPerSample;  /* 34 */
	uint32_t   SubChunk2ID;   /* 36 */
	uint32_t   SubChunk2Size; /* 40 */
}WAV_InfoTypedef ;

void audioInitialization(void);
void audioPlayFile(const char* filename);
uint8_t isAudioPlaying(void);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_Play(uint32_t frequency);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_Stop(void);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_Pause(void);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_Resume(void);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_Mute(uint8_t state);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_GetFileInfo(uint8_t* file, WAV_InfoTypedef* info);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_SelectFile(uint8_t * file);
AUDIOPLAYER_StateTypdef  AUDIOPLAYER_GetState(void);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_DeInit(void);
uint32_t                 AUDIOPLAYER_GetProgress (void);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_SetVolume(uint32_t volume);
uint32_t                 AUDIOPLAYER_GetVolume(void);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_SetPosition(uint32_t position);

AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_NotifyEndOfFile(void);
AUDIOPLAYER_ErrorTypdef  AUDIOPLAYER_NotifyError(void);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_H_ */
