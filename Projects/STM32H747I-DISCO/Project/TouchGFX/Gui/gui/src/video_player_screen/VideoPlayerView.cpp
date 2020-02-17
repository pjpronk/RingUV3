

#include <string.h>
#include <touchgfx/hal/HAL.hpp>

#include <gui/video_player_screen/VideoPlayerView.hpp>

#ifdef _MSC_VER
#define strcat strcat_s
#define strncpy strncpy_s
#endif

#ifndef SIMULATOR
#include <stm32h7xx_hal.h>
#include <stm32h7xx_hal_jpeg.h>

#include "hwjpeg_decode.h"

#include <cmsis_os.h>

struct HwJPEG_Context_s HwJPEG_Context;
#endif

#include "lib/Config.h"
#include "lib/Debug.h"
#include "lib/Audio.h"

MJPEGReader mJPEGReaderMovie;
Box noMovieBackground;
ClickListener<PixelDataWidget> moviePixelWidget;
char* audio_file = NULL;

VideoPlayerView::VideoPlayerView()
    : playMovieButtonPressedCallback(this, &VideoPlayerView::playMovieButtonPressedHandler),
	  movieEndedCallback(this, &VideoPlayerView::onMovieEnded)
{
    movieBufferBitmapId = Bitmap::dynamicBitmapCreate(800, 480, Bitmap::RGB888);
    Bitmap movieBufferBmp = Bitmap(movieBufferBitmapId);
    uint8_t* const movieBuffer = const_cast<uint8_t*>(movieBufferBmp.getData());

    temporaryBufferBitmapId = Bitmap::dynamicBitmapCreate(800, 480, Bitmap::RGB888);
    Bitmap tempBufferBmp = Bitmap(temporaryBufferBitmapId);
    uint8_t* const tempBuffer = const_cast<uint8_t*>(tempBufferBmp.getData());

    //setup movie output
    moviePixelWidget.setPixelData(movieBuffer);
    moviePixelWidget.setBitmapFormat(Bitmap::RGB888);

    //setup movie readers
    mJPEGReaderMovie.setFrameBuffer(movieBuffer);
    mJPEGReaderMovie.setTemporaryFrameBuffer(tempBuffer);
    mJPEGReaderMovie.setLineBuffer(lineBuffer);
    mJPEGReaderMovie.setFileBuffer(fileBuffer, sizeof(fileBuffer));
    mJPEGReaderMovie.setOutputWidget(moviePixelWidget);
    mJPEGReaderMovie.setTimeBetweenTicks(16);
    mJPEGReaderMovie.setMovieEndedAction(movieEndedCallback);
    mJPEGReaderMovie.setRepeat(false);

    mJPEGReaderThumb.setTemporaryFrameBuffer(tempBuffer);
    mJPEGReaderThumb.setLineBuffer(lineBuffer);
    mJPEGReaderThumb.setFileBuffer(fileBuffer, sizeof(fileBuffer));
}

void VideoPlayerView::setupScreen()
{
    JPEG_InitDecode_HW(&HwJPEG_Context);

    //Background for no video selected
    noMovieBackground.setPosition(0, 0, 800, 480);
    noMovieBackground.setColor(0xFFFFFF);
    add(noMovieBackground);

//    playMovieButton.setPosition(0, 0, 800, 480);
//    playMovieButton.setAction(playMovieButtonPressedCallback);
//    add(playMovieButton);

    //PixelDataWidget for movie, fullscreen by default
    moviePixelWidget.setPosition(0, 0, 800, 480);
    moviePixelWidget.setVisible(true);
    add(moviePixelWidget);

    HAL::getInstance()->setFrameRateCompensation(true);
}

void VideoPlayerView::tearDownScreen()
{
    HAL::getInstance()->lockFrameBuffer();
    HAL::getInstance()->unlockFrameBuffer();
    //HAL::getInstance()->setTouchSampleRate(2);

    mJPEGReaderMovie.closeFile();
    fileinput::deInit();

    Bitmap::dynamicBitmapDelete(movieBufferBitmapId);
    Bitmap::dynamicBitmapDelete(temporaryBufferBitmapId);

    HAL::getInstance()->setFrameRateCompensation(false);

    JPEG_StopDecode_HW(&HwJPEG_Context);
}

void VideoPlayerView::onMovieEnded()
{
	if(mJPEGReaderMovie.getRepeat() == true)
	{
		audioPlayFile(audio_file);
	}
}

void VideoPlayerView::onAction(Action action)
{
    switch (action)
    {
    case HW_ON:
        mJPEGReaderMovie.setHWDecodingEnabled(true);
        break;
    case HW_OFF:
        mJPEGReaderMovie.setHWDecodingEnabled(false);
        break;
    case REPEAT_ON:
        mJPEGReaderMovie.setRepeat(true);
        break;
    case REPEAT_OFF:
        mJPEGReaderMovie.setRepeat(false);
        break;
    case PLAY:
//        state = PLAYING;
//        moviePlayButton.invalidate();
//        moviePlayButton.setVisible(false);
//        fpsInfo.setVisible(true);
//        showBars(BAR_FADEOUT);
        break;
    case PAUSE:
      {
        mJPEGReaderMovie.pause();
//        moviePlayButton.setVisible(true);
//        moviePlayButton.invalidate();
        moviePixelWidget.setVisible(true);
        moviePixelWidget.invalidate();
        Rect r(0, 0, 800, 480);
        HAL::lcd().copyFrameBufferRegionToMemory(r);
//        state = PAUSED;
//        showBars(BAR_FADEIN);
        break;
      }
    case REPEAT:
        break;
    case PLAYLIST:
//        playlistWidget.setVisible(true);
//        playlistWidget.invalidate();
//        topBar.setText("PLAYLIST");
//        showBars(BAR_TOP_ONLY);
        break;
    case PLAYLIST_BACK:
        break;
    case FOLDER:
        {
            moviePixelWidget.setTouchable(false);
//            uint32_t n = fileinput::getDirectoryList(currentFolder, ".AVI", dirlist, DIRLIST_LENGTH);
//            folderWidget.setupDirlist(currentFolder, dirlist, n);
//            folderWidget.moveTo(0, 70);
//            showBars(BAR_TOP_ONLY);
            break;
        }
    default:
        assert(!"Unknown action!");
    }
}

void VideoPlayerView::handleTickEvent()
{
    mJPEGReaderMovie.tick();
}

void VideoPlayerView::playMovieButtonPressedHandler(const AbstractButton& src)
{
//    char video[] = "0://VIDEO/OCEAN.AVI";
//
//    playFile(video);
}

void VideoPlayerView::handleClickEvent(const ClickEvent& evt)
{
    Screen::handleClickEvent(evt);
}

void VideoPlayerView::screenClick(ClickEvent::ClickEventType type, int x, int y)
{
    ClickEvent temp_evt(type, x, y);
    handleClickEvent(temp_evt);
}

void VideoPlayerView::playFile(const char* filename)
{
//	mJPEGReaderMovie.closeFile();
//
//    fileinput::FileHdl f = fileinput::openFile(filename);
//    if (mJPEGReaderMovie.setAVIFile(f))
//    {
//        mJPEGReaderMovie.showFirstFrame();
//
//        mJPEGReaderMovie.play();
//    }
}

void playAviFile(const char* video_filename, bool repeat, const char* audio_filename)
{
//	if(mJPEGReaderMovie.isPlaying() == true)
//	{
		mJPEGReaderMovie.setRepeat(false);
//		mJPEGReaderMovie.pause();
//	}

	mJPEGReaderMovie.closeFile();

	osDelay(50);

    fileinput::FileHdl f = fileinput::openFile(video_filename);
    if (mJPEGReaderMovie.setAVIFile(f))
    {
        moviePixelWidget.invalidate();
        noMovieBackground.setVisible(false);

        mJPEGReaderMovie.showFirstFrame();

        osDelay(50);

        mJPEGReaderMovie.play();
        mJPEGReaderMovie.setRepeat(repeat);
        audio_file = (char*)audio_filename;

        osDelay(50);

        audioPlayFile(audio_filename);
    }
}

void pauseAviFile(bool immediately)
{
	if(immediately)
		mJPEGReaderMovie.pause();

	mJPEGReaderMovie.setRepeat(false);
}


uint8_t isVideoPlaying(void)
{
	return (mJPEGReaderMovie.isPlaying() == true);
}
