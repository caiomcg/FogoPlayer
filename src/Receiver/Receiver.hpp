#pragma once

#include <linux/socket.h>

#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "SDLWrapper.hpp"

#include "../InputMedia/LibAV/LibAVInputMedia.hpp"

extern "C" {
    #include <libavutil/imgutils.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
}

class Receiver : public SDLEventListener {
private:
    bool keep_alive_;
    int socket_fd_;
    int border_offset_;

    bool should_pause_;
    std::mutex pause_mutex_;
    std::condition_variable paused_condition_;

    std::string quadrant_;

    LibAVInputMedia* input_media_;
public:
    Receiver(const std::string& quadrant);

    void setInputMedia(LibAVInputMedia* input_media);
    void setBorderOffset(int border_offset);
    void onWindowClosed() override;
    void onPausePressed(bool state) override;

    void run(const std::string& socket_info);
    std::thread spawn(const std::string& socket_info); 
};