#pragma once

#include <memory>
#include <thread>

#include "../InputMedia/InputMedia.hpp"
#include "../InputMedia/OpenCV/OpencvInputMedia.hpp"

class Synchronizer {
private:
    std::unique_ptr<OpencvInputMedia> input_media_;
public:
    void run(const std::string& file);
    std::thread spawn(const std::string& file);
    Synchronizer& setInputMedia(std::unique_ptr<OpencvInputMedia> input);
};