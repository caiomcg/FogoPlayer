#pragma once

#include <linux/socket.h>

#include <string>
#include <thread>

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
    LibAVInputMedia* input_media_;
public:
    Receiver();

    void setInputMedia(LibAVInputMedia* input_media);
    void onWindowClosed() override;

    void run(const std::string& socket_info);
    std::thread spawn(const std::string& socket_info); 
};