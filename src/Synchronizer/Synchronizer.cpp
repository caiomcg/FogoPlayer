#include "Synchronizer.hpp"

#include <iostream>
#include <algorithm>

Synchronizer::Synchronizer() : qr_code_finder_() {}

void Synchronizer::run(const std::string& file) {
    std::cout << "Synchronizer is running" << std::endl;
    this->input_media_->open(file);
    bool keep_alive = true; // FIXME: Should be atomic global
    
    while (keep_alive) {
        cv::Mat frame = this->input_media_->read();
        auto qr_data = this->qr_code_finder_.process(frame);
        
        for (auto qr_info : qr_data) {
            #ifdef DEBUG
            auto x1 = std::min(qr_info.location.at(0).x, std::min(qr_info.location.at(1).x, std::min(qr_info.location.at(2).x, qr_info.location.at(3).x))); // top-left pt. is the leftmost of the 4 points
            auto x2 = std::max(qr_info.location.at(0).x, std::max(qr_info.location.at(1).x, std::max(qr_info.location.at(2).x, qr_info.location.at(3).x))); // bottom-right pt. is the rightmost of the 4 points
            auto y1 = std::min(qr_info.location.at(0).y, std::min(qr_info.location.at(1).y, std::min(qr_info.location.at(2).y, qr_info.location.at(3).y))); //top-left pt. is the uppermost of the 4 points
            auto y2 = std::max(qr_info.location.at(0).y, std::max(qr_info.location.at(1).y, std::max(qr_info.location.at(2).y, qr_info.location.at(3).y))); //bottom-right pt. is the lowermost of the 4 points
            cv::rectangle(frame, cv::Point(x1,y1), cv::Point(x2,y2), cv::Scalar(0, 255, 0));
            #endif
            std::cout << qr_info.data << " ";
        }
        std::cout << std::endl;
        
        
        #ifdef DEBUG
        cv::imshow("frame", frame);
        if(cv::waitKey(1) == 27)
            break;
        #endif
    }
}

std::thread Synchronizer::spawn(const std::string& file) {
    return std::thread(&Synchronizer::run, this, file); 
}

Synchronizer& Synchronizer::setInputMedia(std::unique_ptr<OpencvInputMedia> input) {
    this->input_media_ = std::move(input);
    return *this;
}
