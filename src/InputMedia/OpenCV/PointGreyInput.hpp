#pragma once

#include "OpencvInputMedia.hpp"
#include "FlyCapture2.h"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

class PointGreyInput : public OpencvInputMedia {
private:
    cv::VideoCapture video_source_;
    FlyCapture2::Camera camera_;
    FlyCapture2::CameraInfo camera_info_;
    FlyCapture2::Error error_;
public:
    virtual ~PointGreyInput() override;

    cv::Mat read() override;
    void open(const std::string& file_name) override;
};