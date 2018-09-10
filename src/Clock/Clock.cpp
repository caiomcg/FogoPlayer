#include "Clock.h"

#include <iostream>
#include <thread>

#define BIT_SET(data, bit) ((data & bit) == bit)

Clock::Clock(const int& port) {
    std::clog << "Connecting to port: " << port << std::endl;
    this->sockaddr_size_ = sizeof(this->last_message_info_);

    if ((this->fd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        throw std::runtime_error("Could not create socket");
    }

    this->sock_info_.sin_family = AF_INET;    
    this->sock_info_.sin_addr.s_addr = INADDR_ANY;
    this->sock_info_.sin_port = htons(port); 


    int reuse = 1;
    if (setsockopt(this->fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "Failed to set reuse" << std::endl;
    }

    if (bind(this->fd_, (struct sockaddr*)&this->sock_info_, sizeof(this->sock_info_)) == -1) {
        throw std::runtime_error("Could not bind");
    }

    if (listen(this->fd_, 1) == -1) {
        throw std::runtime_error("Could not listen");
    }
}

Clock::~Clock() {
    this->observers_.clear();
}

void Clock::setObserver(ClockObserver* observer) {
    this->observers_.push_back(observer);
}

int Clock::presentationCotrol() {
    if (drop_ammount < 0) {
        return drop_ammount++;
    }
    
    if (drop_ammount > 0) {
        return drop_ammount--;
    }

    return 0;
}

int Clock::ptsToRealClock(AVFrame* frame, const int& q2d) {
    pts = frame->pts * q2d; // PTS in seconds

    double frame_delay;

    if(pts != 0) {
        /* if we have pts, set video clock to it */
        video_clock = pts;
    } else {
        /* if we aren't given a pts, set it to the clock */
        pts = video_clock;
    }
    /* update the video clock */
    frame_delay = q2d;

    /* if we are repeating a frame, adjust clock accordingly */
    frame_delay += frame->repeat_pict * (frame_delay * 0.5);
    video_clock += frame_delay;

    double delay = pts - last_pts;
    if(delay <= 0 || delay >= 1.0) {
        delay = last_delay;
    }
    /* save for next time */
    last_delay = delay;
    last_pts = pts;

    frame_timer += delay;
    
    actual_delay = frame_timer - (av_gettime() / 1000000.0);
    if(actual_delay < 0.010) { // Use ffplay metric of a minimum of 10ms delay between frames
        actual_delay = 0.010;
    }

    return (int)(actual_delay * 1000 + 0.5);
}

void Clock::start() {
    std::thread(&Clock::run, this).detach();
}

void Clock::stop() {
    keep_alive_ = false;
}


void Clock::process(uint8_t* buffer) {
    for (auto observer : this->observers_) {
        if (observer != nullptr) {
            if (BIT_SET(buffer[0], 0x01)) {
                std::clog << "Show QR" << std::endl;
                observer->showQR(true);
            } else if (BIT_SET(buffer[0], 0x02)) {
                std::clog << "Hide QR" << std::endl;
                observer->showQR(false);
            } else if (BIT_SET(buffer[0], 0x04)) {
                std::clog << "Drop frames" << std::endl;
                this->drop_ammount = 0x00000000;
                this->drop_ammount = ((buffer[1] << 24) | (buffer[2] << 16) | (buffer[3] << 8) | buffer[4]);
                std::clog << "Drop ammount " << this->drop_ammount << std::endl;
            }
        }
    }
}

void Clock::run() {
    int client_fd = 0;
    int bytes_received = 0;

    uint8_t* buffer = new uint8_t[6];


    //FIXME: Should be non blocking
    while (keep_alive_ && (client_fd = accept(this->fd_, (struct sockaddr*)&this->last_message_info_, &this->sockaddr_size_)) != -1) {
        std::clog << "Receiving data from client" << std::endl;

        std::thread([&]() {
            while ((bytes_received = recv(client_fd, buffer, 6, 0)) != -1) {
                std::clog << "Processing" << std::endl;
                if (bytes_received != 0) {
                    this->process(buffer);
                    
                } else {
                    break;
                }
            }
        }).detach();
    }

    delete[] buffer;
}