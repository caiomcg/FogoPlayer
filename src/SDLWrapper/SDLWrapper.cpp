#include "SDLWrapper.hpp"
#include <iostream>

SDLWrapper::SDLWrapper(LibAVInputMedia* input_media, std::shared_ptr<RingQueue<AVFrame*>> decodec_frame_queue) : event_listener(nullptr), codec_ctx_(input_media->getCodecContext()), decodec_frame_queue_(decodec_frame_queue) {
    SDL_Init(SDL_INIT_VIDEO);

    std::cout << "Creating window with size: " << this->codec_ctx_->width << " x " << this->codec_ctx_->height << std::endl;

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1); // TODO: Decide the appropriate opengl options

    if (!(this->window_ = SDL_CreateWindow("FogoPlayer", 0, 0, this->codec_ctx_->width, this->codec_ctx_->height, SDL_WINDOW_RESIZABLE |SDL_WINDOW_OPENGL))) {
        SDL_Quit();
        throw std::runtime_error("Could not initialize SDL window");
    }

    if (!(this->renderer_ = SDL_CreateRenderer(this->window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))) {
        SDL_DestroyWindow(this->window_);
        SDL_Quit();
        throw std::runtime_error("SDL_CreateRenderer Error: " + std::string(SDL_GetError()));
    }

    if (!(this->texture_ = SDL_CreateTexture(this->renderer_, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, this->codec_ctx_->width, this->codec_ctx_->height))) {
        SDL_DestroyWindow(this->window_);
        SDL_Quit();
        throw std::runtime_error("Could not initialize video texture" + std::string(SDL_GetError()));
    }
}

void SDLWrapper::registerListener(SDLEventListener* listener) {
    this->event_listener = listener;
}

void SDLWrapper::run() {
    AVFrame* frame = nullptr;
    AVFrame* RGB_frame = av_frame_alloc();

    SDL_Event evt;

    int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB24, this->codec_ctx_->width, this->codec_ctx_->height, 8);
    if (numBytes < 1) {
        throw std::runtime_error("Could not get image size in bytes");
    }

    uint8_t* frame_buffer = (uint8_t*)av_malloc(numBytes);
    if (!frame_buffer) {
        throw std::runtime_error("Could not allocate image buffer");
    }

    int av_status = av_image_fill_arrays(&RGB_frame->data[0], &RGB_frame->linesize[0], frame_buffer, AV_PIX_FMT_RGB24, this->codec_ctx_->width, this->codec_ctx_->height, 1);
    if (av_status < 0) {
        throw std::runtime_error("Could not fill the RGB frame array");
    }

    SwsContext* sws_context = sws_getContext(this->codec_ctx_->width, this->codec_ctx_->height,
        this->codec_ctx_->pix_fmt, this->codec_ctx_->width, this->codec_ctx_->height,
        AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);

    while ((frame = decodec_frame_queue_->take()) != nullptr) {
        std::clog << "PTS: " << frame->pts << std::endl;

        sws_scale(sws_context, frame->data, frame->linesize, 0, this->codec_ctx_->height, RGB_frame->data, RGB_frame->linesize);
        
        SDL_UpdateTexture(this->texture_, NULL, RGB_frame->data[0], RGB_frame->linesize[0]);
        SDL_RenderClear(this->renderer_);
        SDL_RenderCopy(this->renderer_, this->texture_, NULL, NULL);
        SDL_RenderPresent(this->renderer_);

        av_frame_free(&frame);
        
        SDL_PollEvent(&evt);
        if (evt.type == SDL_QUIT) {
            std::cerr << "quitting" << std::endl;
            SDL_DestroyWindow(this->window_);
            SDL_DestroyRenderer(this->renderer_);
            SDL_DestroyTexture(this->texture_);
            SDL_Quit();
            if (this->event_listener != nullptr) {
                std::thread([this]() { // Let the user handle screen exit
                    this->event_listener->onWindowClosed();
                }).detach();
            }
			break;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(41));
    }

    av_free(frame_buffer);
    av_frame_free(&RGB_frame);
}

std::thread SDLWrapper::spawn() {
    return std::thread(&SDLWrapper::run, this);
}