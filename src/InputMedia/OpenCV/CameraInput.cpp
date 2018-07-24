#include "CameraInput.hpp"

#include <stdexcept>

CameraInput::~CameraInput() {}

cv::Mat CameraInput::read() {
    cv::Mat frame;
    this->video_source_.read(frame);
    return frame;
}

void CameraInput::open(const std::string& file_name) {
    if (!this->video_source_.open(file_name)) {
        throw std::runtime_error("Could not open camera with id: " + file_name);
    }
}