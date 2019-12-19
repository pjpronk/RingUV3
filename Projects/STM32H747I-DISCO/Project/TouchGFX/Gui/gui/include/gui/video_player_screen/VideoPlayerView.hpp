

#ifndef VIDEO_PLAYER_VIEW_HPP
#define VIDEO_PLAYER_VIEW_HPP

#include <touchgfx/Application.hpp>
#include <touchgfx/mixins/ClickListener.hpp>
#include <touchgfx/mixins/MoveAnimator.hpp>
#include <touchgfx/widgets/Image.hpp>
#include <touchgfx/widgets/PixelDataWidget.hpp>
#include <touchgfx/widgets/TouchArea.hpp>

#include <gui/common/DemoView.hpp>
#include <gui/common/Playlist.hpp>
#include <gui/common/TopBar.hpp>
#include <gui/common/CommonDefinitions.hpp>
#include <gui/video_player_screen/VideoBottomBar.hpp>
#include <gui/video_player_screen/FolderWidget.hpp>
#include <gui/video_player_screen/PlaylistWidget.hpp>
#include <gui/video_player_screen/VideoPlayerPresenter.hpp>
#include <gui/video_player_screen/VideoProgressBar.hpp>
#include <gui/video_player_screen/mjpegreader.hpp>

void playAviFile(const char* video_filename, bool repeat, const char* audio_filename);
void pauseAviFile(bool immediately);
uint8_t isVideoPlaying(void);

extern MJPEGReader mJPEGReaderMovie;

extern Box noMovieBackground;
extern ClickListener<PixelDataWidget> moviePixelWidget;

using namespace touchgfx;

class VideoPlayerView : public DemoView<VideoPlayerPresenter>
{
public:
    VideoPlayerView();
    virtual ~VideoPlayerView() { }

    virtual void setupScreen();
    virtual void tearDownScreen();
    virtual void handleTickEvent();
    //Playlist Manager functions
    virtual void handleClickEvent(const ClickEvent& evt);

    void screenClick(ClickEvent::ClickEventType type, int x, int y);
    void playFile(const char* filename);
private:
    Callback<VideoPlayerView> movieEndedCallback;
    Callback<VideoPlayerView, const AbstractButton&> playMovieButtonPressedCallback;

    void onMovieEnded();
    void onAction(Action action);

    void playMovieButtonPressedHandler(const AbstractButton& src);

    //Buffers in internal RAM
    //Buffer to decode one line of JPEG
    uint8_t lineBuffer[RGB24BYTES(800, 1)];
    //RAM for AVI file buffer, 100 Kb, please encode video at max 800 kbits/s
    uint8_t fileBuffer[100 * 1024];

    MJPEGReader mJPEGReaderThumb;

    BitmapId movieBufferBitmapId;
    BitmapId temporaryBufferBitmapId;
};

#endif // VIDEO_PLAYER_VIEW_HPP
