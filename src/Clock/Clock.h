#pragma once

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <vector>

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavutil/time.h>
}

class ClockObserver {
public:
    virtual ~ClockObserver() {}
    virtual void showQR(bool state) = 0;
};

class Clock {
private:
    double pts = 0.0;
    double last_pts = 0.0;
    double video_clock = 0.0;
    double last_delay = 40e-3;
    double frame_timer = (double)av_gettime() / 1000000.0;
    double actual_delay = 0.0;

    int drop_ammount = 0;

    bool keep_alive_ = true;

    int fd_;

    struct sockaddr_in sock_info_;
    struct sockaddr_in last_message_info_;

    socklen_t sockaddr_size_;

    std::vector<ClockObserver*> observers_;
    void process(uint8_t* buffer);
public:
    Clock(const int& port);
    ~Clock();

    int ptsToRealClock(AVFrame* frame, const double& q2d);
    int presentationCotrol();

    void setObserver(ClockObserver* observer);

    void run();
    void start();
    void stop();
};