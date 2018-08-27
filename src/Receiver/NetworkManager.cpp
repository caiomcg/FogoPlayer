#include "NetworkManager.hpp"

#include <iostream>

#define BIT_SET(data, bit) ((data & bit) == bit)

NetworkManager::NetworkManager(const int& port) {
    std::clog << "Connecting to port: " << port << std::endl;
    this->sockaddr_size_ = sizeof(this->last_message_info_);

    if ((this->fd_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        throw std::runtime_error("Could not create socket");
    }

    this->sock_info_.sin_family = AF_INET;    
    this->sock_info_.sin_addr.s_addr = INADDR_ANY;
    this->sock_info_.sin_port = htons(port); 


    int reuse = 1;
    if (setsockopt(this->fd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        std::cerr << "Failed to set reuse" << std::endl;
    }

    if (bind(this->fd_, (struct sockaddr*)&this->sock_info_, sizeof(this->sock_info_)) == -1) {
        throw std::runtime_error("Could not bind");
    }

    if (listen(this->fd_, 1) == -1) {
        throw std::runtime_error("Could not listen");
    }
}

NetworkManager::~NetworkManager() {
}

void NetworkManager::setObserver(NetworkObserver* observer) {
    this->observers_.push_back(observer);
}

void NetworkManager::notifyObservers(const uint8_t& mask) {
    for (auto observer : this->observers_) {
        if (observer != nullptr) {
            if (BIT_SET(mask, 0x01)) {
                std::clog << "play" << std::endl;
                observer->shouldReproduce(true);
            } else if(BIT_SET(mask, 0x02)) {
                std::clog << "Pause" << std::endl;
                observer->shouldReproduce(false);
            } else if (BIT_SET(mask, 0x04)) {
                std::clog << "Show QR" << std::endl;
                observer->showQR(true);
            } else if (BIT_SET(mask, 0x08)) {
                std::clog << "Hide QR" << std::endl;
                observer->showQR(false);
            }
        }
    }
}

void NetworkManager::run() {
    int client_fd = 0;
    int bytes_received = 0;

    uint8_t* buffer = new uint8_t[10];

    while ((client_fd = accept(this->fd_, (struct sockaddr*)&this->last_message_info_, &this->sockaddr_size_)) != -1) {
        std::clog << "Receiving data from client" << std::endl;

        std::thread([&]() {
            while ((bytes_received = recv(client_fd, buffer, 10, 0)) != -1) {
                std::clog << "Processing" << std::endl;
                if (bytes_received != 0) {
                    this->notifyObservers(buffer[0]);
                } else {
                    break;
                }
            }
        }).detach();
    }

    delete[] buffer;
}

std::thread NetworkManager::spawn() {
    return std::thread(&NetworkManager::run, this);
}