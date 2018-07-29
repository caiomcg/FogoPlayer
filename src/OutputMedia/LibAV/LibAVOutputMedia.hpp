#pragma once

#include <thread>
#include <iostream>
#include <memory>

#include "RingQueue.h"
#include "LibAVInputMedia.hpp"

extern "C" {
    #include <libavformat/avformat.h>
    #include <libavcodec/avcodec.h>
}

class LibAVOutputMedia {
protected:
    std::shared_ptr<RingQueue<AVPacket*>> raw_packet_queue_;
    std::shared_ptr<RingQueue<AVFrame*>> raw_output_queue_;

    AVFormatContext* format_ctx_;
    AVCodecContext* codec_ctx_;
    AVCodec* codec_;
    AVStream* stream_;
public:
    virtual ~LibAVOutputMedia() {}

    AVFormatContext* getFormatContext() const {
        return format_ctx_;
    }

    AVCodecContext* getCodecContext() const {
        return codec_ctx_;
    }

    AVCodec* getCodec() const {
        return codec_;
    }

    AVStream* getBestStream() const {
        return stream_;
    }

    std::thread spawn() {
        return std::thread(&LibAVOutputMedia::run, this); 
    }

    void setInputQueue(std::shared_ptr<RingQueue<AVPacket*>> input_queue) {
        this->raw_packet_queue_ = input_queue;
    }

    void setOutputQueue(std::shared_ptr<RingQueue<AVFrame*>> output_queue) {
        this->raw_output_queue_ = output_queue;
    }

    void setInputMedia(LibAVInputMedia* input_media) {
        this->codec_ctx_ = input_media->getCodecContext();
    }

    virtual void run() = 0;
    virtual AVFrame* decode() = 0;
};