#include "Synchronizer.hpp"

#include <iostream>

void Synchronizer::run(const std::string& file) {
    this->input_media_->open(file);
    
    while (true) {
        cv::Mat image = this->input_media_->read();
        cv::imshow("frame", image);
        cv::waitKey(1);
    }
}

std::thread Synchronizer::spawn(const std::string& file) {
    return std::thread(&Synchronizer::run, this, file); 
}

Synchronizer& Synchronizer::setInputMedia(std::unique_ptr<OpencvInputMedia> input) {
    this->input_media_ = std::move(input);
    return *this;
}
