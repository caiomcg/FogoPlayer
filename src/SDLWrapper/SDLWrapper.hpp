#pragma once

#include <stdexcept>
#include <thread>
#include <chrono>
#include <memory>

#include <qrencode.h>

#include <SDL2/SDL.h>

#include "RingQueue.h"
#include "Clock.h"
#include "LibAVInputMedia.hpp"

extern "C" {
    #include <libavutil/imgutils.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
    #include <libavutil/time.h>
}

class SDLEventListener {
public:
    virtual void onWindowClosed() = 0;
    virtual void onPausePressed(bool state) = 0;
};

class SDLWrapper : public ClockObserver {
private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
    SDL_Texture* qr_texture_;
    SDLEventListener* event_listener;
    QRcode* qr_code_;
    Clock clock_;

    SDL_Rect video_rect_;

    bool is_playing_;
    bool keep_alive_;
    bool show_qr_;

    int border_offset_;
    double q2d_;

    std::string quadrant_;

    AVCodecContext* codec_ctx_;

    SDLEventListener* l;

    std::shared_ptr<RingQueue<AVFrame*>> decodec_frame_queue_;

    void eventListener();
    void destroy();
    void updateVideoRect(SDL_Rect& rect);

    void showQR(bool state) override;

    void renderAndShow(SDL_Surface* surface, const SDL_Color& color, SDL_Rect& rect);
public:
    SDLWrapper(const std::string& file_name, LibAVInputMedia* input_media, std::shared_ptr<RingQueue<AVFrame*>> decodec_frame_queue, int border_offset = 0);

    void registerListener(SDLEventListener* listener);

    void setQ2d(double q2d);
    void setQuadrant(const std::string& quadrant);

    void run();
    std::thread spawn();
};