#pragma once

#include <memory>
#include <thread>

#include "../InputMedia/InputMedia.hpp"
#include "../InputMedia/OpenCV/OpencvInputMedia.hpp"
#include "../QRCode/QRCodeFinder.hpp"


class Synchronizer {
private:
    QRCodeFinder qr_code_finder_;
    std::unique_ptr<OpencvInputMedia> input_media_;
public:
    Synchronizer();

    void run(const std::string& file);
    std::thread spawn(const std::string& file);
    Synchronizer& setInputMedia(std::unique_ptr<OpencvInputMedia> input);
};