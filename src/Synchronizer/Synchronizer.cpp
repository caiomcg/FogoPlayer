#include "Synchronizer.hpp"

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <algorithm>

#include <regex>

#define THRESHOLD 0

struct Median {
    std::vector<int> values;
    Median() {
        
    }
    int getMedian() {
        size_t size = values.size();
        if (size == 0) {
            return 0;
        }

        std::sort(values.begin(), values.end());
        
        if (size % 2 == 0) {
            return (values[size / 2 - 1] + values[size / 2]) / 2;
        } else {
            return values[size / 2];
        }
    }

    int calculateSD() {
        float sum = 0.0, mean, standardDeviation = 0.0;

        int i;

        for(i = 0; i < values.size(); i++) {
            sum += values[i];
        }

        mean = sum/10;

        for(i = 0; i < values.size(); i++)
            standardDeviation += pow(values[i] - mean, 2);

        return sqrt(standardDeviation / 10);
    }
};

Synchronizer::Synchronizer() : qr_code_finder_() {}

Synchronizer::~Synchronizer() {
    for (auto client : this->clients_) {
        this->sendData(client.first, 0x02, 0x00);
        close(client.second);
    }
}

std::pair<std::string, int> Synchronizer::extractInfo(const std::string& source) {
    std::smatch sm;
    std::regex_match(source, sm, std::regex("Q=(\\d+):PTS=(\\d+)"));
    if (sm.size()) {
        return std::pair<std::string, int>(sm[1], std::stoi(sm[2]));
    }
    return std::pair<std::string, int>("", 0);
}

int Synchronizer::dropCount(const int& base_pts, const int& quadrant_pts) {
    int count = base_pts - quadrant_pts;

    if (count < THRESHOLD || count > THRESHOLD) {
        return count;
    }
    return 0;
}


void Synchronizer::run(const std::string& file) {
    std::cout << "Synchronizer is running" << std::endl;
    this->input_media_->open(file);
    bool keep_alive = true; // FIXME: Should be atomic global

    int i = 0;

    std::thread([this]() {
        int x;
        while (std::cin >> x) {
            for (auto client : this->clients_) {
                this->sendData(client.first, 0x02, 0x00);
            }
        }
    }).detach();

    std::vector<Median> clients_median(this->clients_.size());
    
    while (keep_alive) {
        cv::Mat frame = this->input_media_->read();
        auto qr_data = this->qr_code_finder_.process(frame);

        int base_pts = 0;

        if (qr_data.size() == this->clients_.size()) { // Adicionar moda + desvio padrao
            // Compare and send adjustments
            std::vector<std::pair<std::string, int>> qrs;
            for (auto qr_info : qr_data) {
                auto data_pair = this->extractInfo(qr_info.data);
                if (data_pair.first == "0") { // reference quadrant
                    base_pts = data_pair.second;
                } else {
                    qrs.push_back(data_pair);
                }
            }

            for (auto client : qrs) {
                // Add values to calculate the median
                clients_median[std::stoi(client.first)].values.push_back(this->dropCount(base_pts, client.second));
            }

            if ((i = (i+1) % 50) == 0 ) {
                for (unsigned index = 0; index < clients_median.size(); index++) {
                    auto median_val = clients_median[index].getMedian();
                    std::clog << "Sending info to quadrant " << index << " - median: " << median_val  << std::endl;
                    this->sendData(std::to_string(index), 0x04, median_val);
                    clients_median[index].values.clear();
                }
            }
        }
        

        for (auto qr_info : qr_data) {
            auto x1 = std::min(qr_info.location.at(0).x, std::min(qr_info.location.at(1).x, std::min(qr_info.location.at(2).x, qr_info.location.at(3).x))); // top-left pt. is the leftmost of the 4 points
            auto x2 = std::max(qr_info.location.at(0).x, std::max(qr_info.location.at(1).x, std::max(qr_info.location.at(2).x, qr_info.location.at(3).x))); // bottom-right pt. is the rightmost of the 4 points
            auto y1 = std::min(qr_info.location.at(0).y, std::min(qr_info.location.at(1).y, std::min(qr_info.location.at(2).y, qr_info.location.at(3).y))); //top-left pt. is the uppermost of the 4 points
            auto y2 = std::max(qr_info.location.at(0).y, std::max(qr_info.location.at(1).y, std::max(qr_info.location.at(2).y, qr_info.location.at(3).y))); //bottom-right pt. is the lowermost of the 4 points
            cv::rectangle(frame, cv::Point(x1,y1), cv::Point(x2,y2), cv::Scalar(0, 255, 0));
            std::clog << qr_info.data << " ";
        }

        std::clog << std::endl;
           
        cv::imshow("frame", frame);
        if(cv::waitKey(1) == 27)
            break;
    }
}

std::thread Synchronizer::spawn(const std::string& file) {
    return std::thread(&Synchronizer::run, this, file); 
}

void Synchronizer::registerClients(std::pair<std::string, std::string> clients) {
    this->clients_.insert(std::pair<std::string, int>(clients.first, this->createSocket(clients.second)));
}

int Synchronizer::createSocket(const std::string& ip) {
    int fd = 0;

    struct sockaddr_in sock_info;
    struct sockaddr_in last_message_info;

    socklen_t sockaddr_size_  = sizeof(last_message_info);

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        throw std::runtime_error("Could not create socket");
    }

    sock_info.sin_family = AF_INET;    
    sock_info.sin_addr.s_addr = inet_addr(ip.c_str());
    sock_info.sin_port = htons(8080); 

    int reuse = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "Failed to set reuse" << std::endl;
    }

    if (connect(fd, (struct sockaddr*)& sock_info, sockaddr_size_) == -1) {
        throw std::runtime_error("Could not connect");
    }

    char buffer[5];

    buffer[0] = 0x01;

    if (send(fd, buffer, 5, 0) < 0) {
        perror("Send failed");
    }

    this->sendData("0", 0x01, this->dropCount(0, 0));

    return fd;
}

/**
 * @brief Send data to a quadrant
 * 
 * @param quadrant Quadrant that will receive the data
 * @param command The command to be sent
 *                0x01 - Show QR Code
 *                0x02 - Hide QR Code
 *                0x04 - Delay or drop frames(require frame_delay)
 *                  
 * @param frame_delay Ammount to hold or delay
*                 If a positive integer, the client will drop n frames
*                 If a negative integer, the client will hold the frame by 1/framerate for every negative number
 */
void Synchronizer::sendData(const std::string& quadrant, int command, int frame_delay) {
    for (auto client : clients_) {
        if (quadrant == client.first) {
            buffer_[0] = command;
            buffer_[1] = (frame_delay >> 24) & 0xFF;
            buffer_[2] = (frame_delay >> 16) & 0xFF;
            buffer_[3] = (frame_delay >> 8) & 0xFF;
            buffer_[4] = frame_delay & 0xFF;

            if (send(client.second, buffer_, 5, 0) < 0) {
                perror("Send failed");
            }
        }
    }
}

Synchronizer& Synchronizer::setInputMedia(std::unique_ptr<OpencvInputMedia> input) {
    this->input_media_ = std::move(input);
    return *this;
}
