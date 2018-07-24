#pragma once

#include "OpencvInputMedia.hpp"

class CameraInput : public OpencvInputMedia {
private:
    cv::VideoCapture video_source_;
public:
    virtual ~CameraInput() override;

    cv::Mat read() override;
    void open(const std::string& file_name) override;
};