#include "HardwareDecoder.hpp"

#include "RingQueue.h"

HardwareDecoder::~HardwareDecoder() {}

void HardwareDecoder::run() {
    AVFrame*  frame  = nullptr;

    while ((frame = this->decode()) != nullptr) {
        this->raw_output_queue_->put(&frame);
    }

    this->raw_output_queue_->put(nullptr);
}

AVFrame* HardwareDecoder::decode() {
    AVFrame* frame = av_frame_alloc();
    AVFrame* GPU_frame = av_frame_alloc();
    int av_status = 0;

    AVPacket* packet = nullptr;
    
    while ((packet = this->raw_packet_queue_->take()) != nullptr) {
        if ((av_status = avcodec_send_packet(this->codec_ctx_, packet)) == 0) {
            if ((av_status = avcodec_receive_frame(this->codec_ctx_, frame)) == 0) {
                if (frame->format == AV_PIX_FMT_VAAPI_VLD) {
                    GPU_frame->format = AV_PIX_FMT_NV12;
                } else {
                    // Handle non accelerated pixel format
                }
                //std::clog << "PTS: " << frame->pts << " frame number: " << this->codec_ctx_->frame_number << " cpn: " << frame->coded_picture_number << " dpn: " << frame->display_picture_number << std::endl;
                
                if ((av_status = av_hwframe_transfer_data(GPU_frame, frame, 0)) < 0) {
                    fprintf(stderr, "Error transferring the data to system memory\n");
                    std::cerr << "An error occurred: decoding - " << AVERROR(av_status) << std::endl;
                }
                
                if ((av_status = av_frame_copy_props(GPU_frame, frame)) < 0) {
                    std::cerr << "Failed to copy frame props" << std::endl;
                }
                av_packet_unref(packet);
                av_frame_free(&frame);

                int* frame = new int(this->codec_ctx_->frame_number);
                GPU_frame->opaque = frame;
                return GPU_frame;
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