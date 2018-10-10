#include "SDLWrapper.hpp"
#include <iostream>
#include <qrencode.h>
#include <SDL2/SDL_ttf.h>

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

SDLWrapper::SDLWrapper(const std::string& file_name, LibAVInputMedia* input_media, std::shared_ptr<RingQueue<AVFrame*>> decodec_frame_queue, int border_offset) : event_listener(nullptr), clock_(8080), is_playing_(true), keep_alive_(true), show_qr_(false), border_offset_(border_offset), codec_ctx_(input_media->getCodecContext()), decodec_frame_queue_(decodec_frame_queue) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_ShowCursor(0);
    std::string window_name = file_name + " - FogoPlayer";

    std::cout << "Creating window with size: " << this->codec_ctx_->width << " x " << this->codec_ctx_->height << std::endl;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1); // TODO: Decide the appropriate opengl options

    if (!(this->window_ = SDL_CreateWindow(window_name.c_str(), 0, 0, this->codec_ctx_->width, this->codec_ctx_->height, SDL_WINDOW_FULLSCREEN_DESKTOP |SDL_WINDOW_OPENGL))) {
        SDL_Quit();
        throw std::runtime_error("Could not initialize SDL window");
    }

    if (!(this->renderer_ = SDL_CreateRenderer(this->window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))) {
        SDL_DestroyWindow(this->window_);
        SDL_Quit();
        throw std::runtime_error("SDL_CreateRenderer Error: " + std::string(SDL_GetError()));
    }

    if (!(this->texture_ = SDL_CreateTexture(this->renderer_, SDL_PIXELFORMAT_NV12, SDL_TEXTUREACCESS_STREAMING, this->codec_ctx_->width, this->codec_ctx_->height))) {
        SDL_DestroyWindow(this->window_);
        SDL_Quit();
        throw std::runtime_error("Could not initialize video texture" + std::string(SDL_GetError()));
    }

    if (!(this->qr_texture_ = SDL_CreateTexture(this->renderer_, SDL_PIXELFORMAT_BGR888, SDL_TEXTUREACCESS_STREAMING, 300, 300))) {
        SDL_DestroyWindow(this->window_);
        SDL_Quit();
        throw std::runtime_error("Could not initialize video texture" + std::string(SDL_GetError()));
    }

    this->clock_.setObserver(this);
}

void SDLWrapper::registerListener(SDLEventListener* listener) {
    this->event_listener = listener;
}

void SDLWrapper::run() {
    this->clock_.start();

    AVFrame* frame = nullptr;

    uint8_t* sdl_texture_buffer = nullptr;
    uint8_t* qr_texture_buffer  = nullptr;

    TTF_Init();
    SDL_Color textColor = { 255, 255, 255, 255 };
    SDL_Color hold_color = { 0, 255, 0, 255 };
    SDL_Color drop_color = { 255, 0, 0, 255 };
    SDL_Rect text_rect = {1920 - 500, 1080 - 200, 500, 200};

    TTF_Font* font = TTF_OpenFont("./IBMPlexMono-Regular.ttf", 60);
    if (font == nullptr) {
        throw std::runtime_error("Could not open font");
    }

    int control = 0;

    int window_width = 0;
    int window_height = 0;

    unsigned y_plane_size  = this->codec_ctx_->width * this->codec_ctx_->height;
    unsigned uv_plane_size = (this->codec_ctx_->width * this->codec_ctx_->height) / 2;
    int pitch = 0;

    SDL_GetWindowSize(this->window_, &window_width, &window_height);

    std::thread(&SDLWrapper::eventListener, this).detach();

    SDL_Rect r{0, 0, 1000, 1000};

    this->updateVideoRect(video_rect_);

    uint8_t* temp_buffer = new uint8_t[300*300];
    for (int i = 0; i < 300*300; i++ ) {
        temp_buffer[i] = 0xFF;
    }

    while ((frame = decodec_frame_queue_->take()) != nullptr && this->keep_alive_) {
        int* frame_num = (int*)frame->opaque;
        std::string info = "q=" + this->quadrant_ + ":frame=" + std::to_string(*frame_num);
        delete frame_num;

        if ((control = this->clock_.presentationCotrol()) != 0) {
            if (control > 0) { // Positive integer = drop x frames
                renderAndShow(TTF_RenderText_Solid(font, info.c_str(), drop_color), text_rect);
                av_frame_free(&frame);
                continue;
            }

            while (control < 0) { // Negative integer = hold current frame for the time of x frames
                renderAndShow(TTF_RenderText_Solid(font, info.c_str(), hold_color), text_rect);
                std::this_thread::sleep_for(std::chrono::milliseconds(this->clock_.ptsToRealClock(frame, q2d_)));
                control = this->clock_.presentationCotrol();
            }
        }

        SDL_LockTexture(this->texture_, nullptr, (void **)&sdl_texture_buffer, &pitch);
        
        memcpy(sdl_texture_buffer, frame->data[0], y_plane_size);
        memcpy(sdl_texture_buffer + y_plane_size, frame->data[1], uv_plane_size);

        SDL_UnlockTexture(this->texture_);

        SDL_LockTexture(this->qr_texture_, nullptr, (void **)&qr_texture_buffer, &pitch);

        if ((this->qr_code_ = QRcode_encodeString(info.c_str(), 0, QR_ECLEVEL_L, QR_MODE_8, 0)) != nullptr) {
            int qrwith = this->qr_code_->width * 14;

            for (int i = 2; i < qrwith; i++) {
                for (int j = 2; j < qrwith; j++) {
                    if (this->qr_code_->data[i/14 * this->qr_code_->width + j/14] & 0x01) {
                        temp_buffer[300 * i + j] = 0x00;
                    } else {
                        temp_buffer[300 * i + j] = 0xFF;
                    }
                }
            }      

            int pos = 0;
            int temp_buffer_pos = 0;
            for (int i = 0; i < 300*300*4; i++) {
                qr_texture_buffer[i] = temp_buffer[temp_buffer_pos];
                if (++pos > 3) {
                    pos = 0;
                    temp_buffer_pos +=1;
                }
            }
        }

        SDL_UnlockTexture(this->qr_texture_);

        SDL_RenderClear(this->renderer_);
        SDL_RenderCopy(this->renderer_, this->texture_, NULL, &this->video_rect_);

        auto text_surface = TTF_RenderText_Solid(font, info.c_str(), textColor);
        if (text_surface != nullptr) {
            SDL_Texture* message = SDL_CreateTextureFromSurface(this->renderer_, text_surface);
            SDL_RenderCopy(this->renderer_, message, nullptr, &text_rect);
            SDL_DestroyTexture(message);
            SDL_FreeSurface(text_surface);
        }
        
        if (this->show_qr_) {
            SDL_RenderCopy(this->renderer_, this->qr_texture_, NULL, &r);
        }

		std::this_thread::sleep_for(std::chrono::milliseconds(this->clock_.ptsToRealClock(frame, q2d_)));
        SDL_RenderPresent(this->renderer_);
        av_frame_free(&frame);
    }

    delete[] temp_buffer;

    TTF_CloseFont(font);
    TTF_Quit();

    this->destroy();
    this->clock_.stop();
}

void SDLWrapper::renderAndShow(SDL_Surface* surface, SDL_Rect& rect) {
    if (surface == nullptr) return;

    SDL_RenderClear(this->renderer_);    
    SDL_Texture* message = SDL_CreateTextureFromSurface(this->renderer_, surface);
    SDL_RenderCopy(this->renderer_, message, nullptr, &rect);
    SDL_DestroyTexture(message);
    SDL_FreeSurface(surface);

    SDL_RenderPresent(this->renderer_);
}

void SDLWrapper::eventListener() {
    SDL_Event evt;
    int last_x = 0;
    int last_y = 0;

    while (this->keep_alive_) {
        SDL_WaitEvent(&evt);
        if (evt.type == SDL_QUIT) {
            this->keep_alive_ = false;
        } else if (evt.type == SDL_KEYUP) {
            switch (evt.key.keysym.sym) {
                case SDLK_f: {
                        bool is_fullscreen = SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN;

                        if (!is_fullscreen){
                            SDL_GetWindowPosition(window_, &last_x, &last_y);
                        }

                        SDL_SetWindowFullscreen(window_, is_fullscreen ? 0 : SDL_WINDOW_FULLSCREEN);

                        if (is_fullscreen){
                            SDL_SetWindowPosition(window_, last_x, last_y);
                        }

                        this->updateVideoRect(video_rect_);
                    }
                    break;
                case SDLK_SPACE: {
                        this->is_playing_ = !this->is_playing_;
                        if (this->event_listener != nullptr) {
                            std::thread([this]() { // Let the user handle screen exit
                                this->event_listener->onPausePressed(this->is_playing_);
                            }).detach();
                        }
                    }
                    break;
                case SDLK_ESCAPE:
                    this->keep_alive_ = false;
                    break;

            }
        }
    }
}

void SDLWrapper::destroy() {
    std::clog << "Destroying SDL wrapper" << std::endl;

    SDL_DestroyWindow(this->window_);
    SDL_DestroyRenderer(this->renderer_);
    SDL_DestroyTexture(this->texture_);
    SDL_Quit();
    
    if (this->event_listener != nullptr) {
        std::thread([this]() { // Let the user handle screen exit
            this->event_listener->onWindowClosed();
        }).detach();
    }
}

void SDLWrapper::updateVideoRect(SDL_Rect& rect) {
    int video_width  = 0;
    int video_height = 0;

    SDL_GetWindowSize(this->window_, &video_width, &video_height);

    rect.x = (border_offset_ != 0 ? -(border_offset_ / 2) : 0);
    rect.y =  0;
    rect.w = video_width + border_offset_;
    rect.h = video_height;
}

void SDLWrapper::showQR(bool state) {
    this->show_qr_ = state;
}

void SDLWrapper::setQ2d(double q2d) {
    this->q2d_ = q2d;
}

void SDLWrapper::setQuadrant(const std::string& quadrant) {
    this->quadrant_ = quadrant;
}

std::thread SDLWrapper::spawn() {
    return std::thread(&SDLWrapper::run, this);
}
