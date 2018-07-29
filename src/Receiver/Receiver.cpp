#include "Receiver.hpp"

#include "FileInput.hpp"
#include "SDLWrapper.hpp"
#include "RingQueue.h"
#include "../OutputMedia/LibAV/SoftwareDecoder.hpp"

Receiver::Receiver() : keep_alive_(true), input_media_(new FileInput()), output_media_(new SoftwareDecoder()) {}

void Receiver::setInputMedia(LibAVInputMedia* input_media) { // Not thread safe
    this->input_media_ = input_media;
}

void Receiver::setOutputMedia(LibAVOutputMedia* output_media) {  // Not thread safe
    this->output_media_ = output_media;
}

void Receiver::onWindowClosed() {
    keep_alive_ = false;
}

void Receiver::run(const std::string& socket_info) { //Producer Thread
    // FirstQueue -> From input media to output media for decoding
    // secondQueue -> From OutputMedia to presentation class
    AVPacket* packet = nullptr;
    
    auto raw_packet_queue = std::make_shared<RingQueue<AVPacket*>>(24); // Maximum of 24 fps
    auto raw_frame_queue  = std::make_shared<RingQueue<AVFrame*>>(24); // Maximum of 24 fps

    this->input_media_->open(socket_info);

    SDLWrapper sdl_wrapper{this->input_media_, raw_frame_queue};
    sdl_wrapper.registerListener(this);
    sdl_wrapper.spawn().detach();
    
    this->output_media_->setInputMedia(this->input_media_);
    this->output_media_->setInputQueue(raw_packet_queue);
    this->output_media_->setOutputQueue(raw_frame_queue);
    this->output_media_->spawn().detach(); // Spawn the first consumer - decoder
    
    while ((packet = this->input_media_->read()) != nullptr && keep_alive_) {
        raw_packet_queue->put(packet);
    }

    raw_packet_queue->put(nullptr); // Destroy the output media
}

std::thread Receiver::spawn(const std::string& file) {
    return std::thread(&Receiver::run, this, file); 
}