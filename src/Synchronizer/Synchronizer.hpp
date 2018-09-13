#pragma once

#include <memory>
#include <thread>
#include <map>

#include "../InputMedia/InputMedia.hpp"
#include "../InputMedia/OpenCV/OpencvInputMedia.hpp"
#include "../QRCode/QRCodeFinder.hpp"


class Synchronizer {
private:
    QRCodeFinder qr_code_finder_;
    std::unique_ptr<OpencvInputMedia> input_media_;
    std::map<std::string, int> clients_;

    uint8_t buffer_[5];

    std::pair<std::string, int> extractInfo(const std::string& source);
    int dropCount(const int& base_pts, const int& quadrant_pts);

public:
    Synchronizer();
    ~Synchronizer();

    void run(const std::string& file);
    std::thread spawn(const std::string& file);

    void registerClients(std::pair<std::string, std::string> clients);
    int createSocket(const std::string& ip);
    void sendData(const std::string& quadrant, int command, int frame_delay);

    Synchronizer& setInputMedia(std::unique_ptr<OpencvInputMedia> input);
};