#include "SoftwareDecoder.hpp"

#include "RingQueue.h"

SoftwareDecoder::~SoftwareDecoder() {}

void SoftwareDecoder::run() {
    AVFrame*  frame  = nullptr;

    while ((frame = this->decode()) != nullptr) {
        this->raw_output_queue_->put(&frame);
    }

    this->raw_output_queue_->put(nullptr);
}

AVFrame* SoftwareDecoder::decode() {
    AVFrame* frame = av_frame_alloc();
    int av_status = 0;

    AVPacket* packet = nullptr;
    
    while ((packet = this->raw_packet_queue_->take()) != nullptr) {
        if ((av_status = avcodec_send_packet(this->codec_ctx_, packet)) == 0) {
            if ((av_status = avcodec_receive_frame(this->codec_ctx_, frame)) == 0) {
                av_packet_unref(packet);
                return frame;
            } else {
                if (av_status == AVERROR(EAGAIN)) {
                    av_packet_unref(packet);
                    continue;
                }
                std::cerr << "An error occurred: decoding - " << AVERROR(EAGAIN) << std::endl;
                break;
            }
        } else {
            std::cerr << "An error occurred: sending to decoder" << std::endl;
            break;
        }
    }


    return nullptr;
}