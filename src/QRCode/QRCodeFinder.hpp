#pragma once

#include <iostream>
#include <vector>
#include <string>

#include <zbar.h>

#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class QRCodeFinder {
private:
    zbar::ImageScanner image_scanner_;
    cv::Mat frame_;

    cv::Mat toGrayscale(const cv::Mat& matrix);
public:
    struct QRCodeData {
        std::string data;
        std::vector <cv::Point> location;
    };

    QRCodeFinder();
    
    std::vector<QRCodeData> process(cv::Mat& frame);
};

std::ostream& operator <<(std::ostream& stream, const QRCodeFinder::QRCodeData& qr_code_data);