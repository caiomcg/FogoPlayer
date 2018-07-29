#include "FileInput.hpp"

#include <stdexcept>
#include <exception>
#include <iostream>

FileInput::~FileInput() {}

void FileInput::open(const std::string& file_name) {
    char error_message[100];
    int error_code = 0;

	avformat_network_init();

    std::clog << "Opening input: " << file_name << std::endl;
    if ((error_code = avformat_open_input (&this->format_ctx_, file_name.c_str(), nullptr, nullptr)) != 0) { // Open the file
        av_strerror(error_code, error_message, 100);
        throw std::runtime_error("OpenPCM::Cold not open input stream " + file_name + " - " + error_message); // Throw an exception if failed
    }

    std::clog << "Finding stream info" << std::endl;
    if (avformat_find_stream_info(this->format_ctx_, nullptr) < 0) { // Find the best stream information
        throw std::runtime_error("Could not fetch stream info"); // Throw an exception if failed
    }

    av_dump_format(this->format_ctx_, 0, "Teste", 0);
    
    int pcm_stream_index = av_find_best_stream(this->format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, &(this->codec_), 0); // Search for the best stream and store its index

    if (pcm_stream_index < 0) {
        av_strerror(pcm_stream_index, error_message, 100);                    // Fetch the error code
        throw std::runtime_error("Could best stream id not found: " + std::string(error_message));  // Throw an exception if failed
    }
    
    this->stream_ = this->format_ctx_->streams[pcm_stream_index];  // Get the best stream

    if(!(this->codec_ = avcodec_find_decoder(this->stream_->codecpar->codec_id))) {  // Find decoder codec
        throw std::runtime_error("Could find decoder codec.");
    }

    if(!(this->codec_ctx_ = avcodec_alloc_context3(this->codec_))) { // Alloc codec context
        throw std::runtime_error("Could alloc codec context.");
    }

    if ((error_code = avcodec_parameters_to_context(this->codec_ctx_, this->stream_->codecpar)) < 0) {
        av_strerror(error_code, error_message, 100);                                                       // Fetch the error code
        throw std::runtime_error("Could copy codec context to codec parameters of input stream: " + std::string(error_message));  // Throw an exception if failed
    }

    this->codec_ctx_->strict_std_compliance = -2;

    if (avcodec_open2(this->codec_ctx_, this->codec_, nullptr) < 0) { // Open the codec
        throw std::runtime_error("Could not open codec"); // Throw an exception if failed
    }
}

AVPacket* FileInput::read() {
    int status_code = 0;

    AVPacket* packet = av_packet_alloc();

    if ((status_code = av_read_frame(this->format_ctx_, packet)) == 0) { // Blocking
        return packet;
    }

    av_packet_free(&packet);
    return nullptr;
}