#include "PointGreyInput.hpp"

PointGreyInput::~PointGreyInput(){}

void PointGreyInput::open(const std::string& file_name) {
    
    if ((error_ = camera_.Connect(0)) != FlyCapture2::PGRERROR_OK) {
        throw std::runtime_error("Failed to connect to camera");    
    }

    // Get the camera info and print it out
    if ((error_ = camera_.GetCameraInfo(&camera_info_)) != FlyCapture2::PGRERROR_OK) {
        throw std::runtime_error("Could not get camera info");
    }
    
    std::cout << camera_info_.vendorName << " "
              << camera_info_.modelName << " " 
              << camera_info_.serialNumber << std::endl;
	
    if ((error_ = camera_.StartCapture()) == FlyCapture2::PGRERROR_ISOCH_BANDWIDTH_EXCEEDED) {
        throw std::runtime_error("Bandwidth exceeded");
    } else if (error_ != FlyCapture2::PGRERROR_OK) {
        throw std::runtime_error("Failed to start capture");
    } 
}

cv::Mat PointGreyInput::read() {
    std::clog << "Retreiving image" << std::endl;
    FlyCapture2::Image raw_image_;
    FlyCapture2::Image rgb_image_;

    if ((error_ = camera_.RetrieveBuffer( &raw_image_ )) != FlyCapture2::PGRERROR_OK) {
        std::clog << "Error fething, sending empty image" << std::endl;
        return cv::Mat();
    }

    raw_image_.Convert(FlyCapture2::PIXEL_FORMAT_BGR, &rgb_image_);

    // convert to OpenCV Mat
    unsigned int rowBytes = (double)rgb_image_.GetReceivedDataSize()/(double)rgb_image_.GetRows();     
    std::clog << "Returning image" << std::endl;  
    std::cout << "Image size: " << rgb_image_.GetRows() << "x" << rgb_image_.GetCols() << std::endl;   

    unsigned char* image = new unsigned char[rgb_image_.GetDataSize()];
    memcpy(image, rgb_image_.GetData(), rgb_image_.GetDataSize());  

    return cv::Mat(rgb_image_.GetRows(), rgb_image_.GetCols(), CV_8UC3, image, rowBytes);
}