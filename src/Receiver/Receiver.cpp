#include "Receiver.hpp"

#include "FileInput.hpp"
#include "SDLWrapper.hpp"
#include "RingQueue.h"

#include "SoftwareDecoder.hpp"
#include "HardwareDecoder.hpp"

#include <iostream>

Receiver::Receiver() : keep_alive_(true), input_media_(new FileInput()) {}

void Receiver::setInputMedia(LibAVInputMedia* input_media) { // Not thread safe
    this->input_media_ = input_media;
}

void Receiver::onWindowClosed() {
    keep_alive_ = false;
}

void Receiver::run(const std::string& socket_info) { //Producer Thread
    // FirstQueue -> From input media to output media for decoding
    // secondQueue -> From OutputMedia to presentation class
    AVPacket* packet = nullptr;
    auto raw_frame_queue  = std::make_shared<RingQueue<AVFrame*>>(60); // Maximum of 60 fps
    auto raw_packet_queue = std::make_shared<RingQueue<AVPacket*>>(60);

    this->input_media_->open(socket_info);

    SDLWrapper sdl_wrapper{this->input_media_, raw_frame_queue};
    sdl_wrapper.registerListener(this);
    sdl_wrapper.spawn().detach();

    LibAVOutputMedia* output_media_ = new HardwareDecoder();

    output_media_->setInputMedia(this->input_media_);
    output_media_->setInputQueue(raw_packet_queue);
    output_media_->setOutputQueue(raw_frame_queue);
    output_media_->spawn().detach(); // Spawn the first consumer - decoder


    while ((packet = this->input_media_->read()) != nullptr && keep_alive_) {
        if (packet->stream_index == 0) {
            raw_packet_queue->put(&packet);
            std::this_thread::sleep_for(std::chrono::milliseconds(41)); // limit to approximately 24fps
        }
    }
}

std::thread Receiver::spawn(const std::string& file) {
    return std::thread(&Receiver::run, this, file); 
}