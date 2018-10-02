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
    // FIXME: Below is experimental and should be removed eventually
    this->video_source_.set(cv::CAP_PROP_FRAME_WIDTH,860);
    this->video_source_.set(cv::CAP_PROP_FRAME_HEIGHT,480);
    this->video_source_.set(cv::CAP_PROP_FPS, 60);
    this->video_source_.set(cv::CAP_PROP_BUFFERSIZE, 1);

    std::clog << "Buffer size: " << this->video_source_.get(cv::CAP_PROP_BUFFERSIZE) << std::endl;
}