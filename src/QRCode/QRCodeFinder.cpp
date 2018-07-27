#include "QRCodeFinder.hpp"

QRCodeFinder::QRCodeFinder() {
    this->image_scanner_.set_config(zbar::ZBAR_QRCODE, zbar::ZBAR_CFG_ENABLE, 1);
}

std::ostream& operator <<(std::ostream& stream, const QRCodeFinder::QRCodeData& qr_code_data) {
    stream << "QRCodeData:\n"
           << "\t- Data: " << qr_code_data.data << "\n"
           << "\t- Points: \n";
    for (cv::Point point : qr_code_data.location) {
        stream << "\t" << point;
    }
    return stream;
}

cv::Mat QRCodeFinder::toGrayscale(const cv::Mat& matrix) {
    cv::Mat grayscale;
    cv::cvtColor(matrix, grayscale, cv::COLOR_BGR2GRAY);
    return grayscale;
}

std::vector<QRCodeFinder::QRCodeData> QRCodeFinder::process(cv::Mat& frame) {
    std::vector<QRCodeFinder::QRCodeData> qr_list;
    
    cv::Mat grayscale_frame = this->toGrayscale(frame);     
    zbar::Image zbar_image(grayscale_frame.cols, grayscale_frame.rows, "Y800", (uchar *)grayscale_frame.data, grayscale_frame.cols * grayscale_frame.rows);
    
    // Scan the image for barcodes and QRCodes
    if (this->image_scanner_.scan(zbar_image) == -1) {
        return qr_list;
    }

    for (zbar::Image::SymbolIterator symbol = zbar_image.symbol_begin(); symbol != zbar_image.symbol_end(); ++symbol) {
        QRCodeData qr_code_data;
        
        qr_code_data.data = symbol->get_data();
        
        for(int i = 0; i< symbol->get_location_size(); i++) {
            qr_code_data.location.push_back(cv::Point(symbol->get_location_x(i),symbol->get_location_y(i)));
        }
        
        qr_list.push_back(qr_code_data);
    }

    return qr_list;
}
