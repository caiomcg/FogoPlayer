#pragma once

#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdexcept>
#include <cstring>
#include <netinet/in.h>
#include <thread>

class NetworkObserver {
public:
    virtual ~NetworkObserver() {}
    virtual void showQR(bool state) = 0;
    virtual void shouldReproduce(bool state) = 0;
};

class NetworkManager {
private:
    int fd_;

    struct sockaddr_in sock_info_;
    struct sockaddr_in last_message_info_;

    socklen_t sockaddr_size_;

    std::vector<NetworkObserver*> observers_;

    void notifyObservers(const uint8_t& mask);
public:
    NetworkManager(const int& port);
    ~NetworkManager();

    void setObserver(NetworkObserver* observer);

    void run();
    std::thread spawn(); 
};