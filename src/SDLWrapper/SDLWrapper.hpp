#pragma once

#include <stdexcept>
#include <thread>
#include <chrono>
#include <memory>

#include <qrencode.h>

#include <SDL2/SDL.h>

#include "RingQueue.h"
#include "LibAVInputMedia.hpp"

extern "C" {
    #include <libavutil/imgutils.h>
    #include <libavcodec/avcodec.h>
    #include <libswscale/swscale.h>
}

class SDLEventListener {
public:
    virtual void onWindowClosed() = 0;
    virtual void onPausePressed(bool state) = 0;
};

class SDLWrapper {
private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    SDL_Texture* texture_;
    SDL_Texture* qr_texture_;
    SDLEventListener* event_listener;
    QRcode* qr_code_;

    bool is_playing_;
    bool keep_alive_;

    AVCodecContext* codec_ctx_;

    SDLEventListener* l;

    std::shared_ptr<RingQueue<AVFrame*>> decodec_frame_queue_;

    void eventListener();
    void destroy();
public:
    SDLWrapper(const std::string& file_name, LibAVInputMedia* input_media, std::shared_ptr<RingQueue<AVFrame*>> decodec_frame_queue);

    void registerListener(SDLEventListener* listener);

    void run();
    std::thread spawn();
};