#include "Receiver.hpp"

#include "FileInput.hpp"
#include "SDLWrapper.hpp"
#include "RingQueue.h"

#include "HardwareDecoder.hpp"

#include <iostream>

Receiver::Receiver() : keep_alive_(true), border_offset_(0), should_pause_(false), input_media_(new FileInput()) {}

void Receiver::setInputMedia(LibAVInputMedia* input_media) { // Not thread safe
    this->input_media_ = input_media;
}

void Receiver::setBorderOffset(int border_offset) {
    this->border_offset_ = border_offset;
}

void Receiver::onWindowClosed() {
    keep_alive_ = false;
}

void Receiver::onPausePressed(bool state) {
    std::unique_lock<std::mutex> pause_lock(this->pause_mutex_);
    if (state) {
        this->should_pause_ = false;
        this->paused_condition_.notify_one();
    } else {
        this->should_pause_ = true;
    }
}

void Receiver::run(const std::string& socket_info) { //Producer Thread
    // FirstQueue -> From input media to output media for decoding
    // secondQueue -> From OutputMedia to presentation class
    AVPacket* packet = nullptr;

    auto raw_frame_queue  = std::make_shared<RingQueue<AVFrame*>>(60); // Maximum of 60 fps
    auto raw_packet_queue = std::make_shared<RingQueue<AVPacket*>>(60);

    this->input_media_->open(socket_info);

    SDLWrapper sdl_wrapper{socket_info, this->input_media_, raw_frame_queue, this->border_offset_};
    
    NetworkManager network_manager{8080};
    network_manager.setObserver(&sdl_wrapper);
    network_manager.spawn().detach();

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
            std::this_thread::sleep_for(std::chrono::milliseconds(30));

            while (this->should_pause_) {
                std::unique_lock<std::mutex> pause_lock(this->pause_mutex_);
                this->paused_condition_.wait(pause_lock);
            }
        }
    }
}

std::thread Receiver::spawn(const std::string& file) {
    return std::thread(&Receiver::run, this, file);
}